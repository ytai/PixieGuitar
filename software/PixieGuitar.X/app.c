#include "app.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "analog.h"
#include "audio_proc.h"
#include "chain.h"
#include "display.h"
#include "gfx.h"
#include "imu.h"
#include "knob.h"
#include "prng.h"
#include "sync.h"
#include "ticker.h"
#include "time.h"
#include "title_bar.h"

#define CMD_NOP         0x0000
#define CMD_TICK        0x0001
#define CMD_PUSH_APP    0x0002
#define CMD_POP_APP     0x0003
#define CMD_SWITCH_APP  0x0004
#define CMD_UNMASK_TICK 0x0005

static App * active_app = NULL;
static uint8_t knob_prev_pos;
static bool knob_prev_pressed;
static uint8_t prev_soc = 0;
static bool force_redraw = true;
static bool screen_flipped = false;

static TaskHandle_t app_task;
static void * command_queue;
static unsigned tick_mask_count = 0;

static TitleBar title_bar;

static uint8_t chain[30];

static inline float clamp(float value, float min, float max) {
  if (value > max) return max;
  if (value < min) return min;
  return value;
}

static void AudioEvent(int16_t * buffer) {
  AppCommand cmd = { CMD_TICK, (uint16_t) buffer };
  bool result = QueuePushBackFromISR(command_queue, &cmd);
  assert(result);
}

static void TickerEvent() {
  AudioEvent(NULL);
}

static void MaskTick() {
  ++tick_mask_count;
  AppCommand cmd = { CMD_UNMASK_TICK };
  NONBLOCKING() AppPostCommand(cmd);
}

static void UnmaskTick() {
  assert(tick_mask_count);
  --tick_mask_count;
}

static void ResumeApp() {
  if (active_app->OnResume) active_app->OnResume(active_app);

  if (active_app->_flags & APP_EV_MASK_AUDIO) {
    AnalogStart(AudioEvent);
  } else {
    TickerStart(TickerEvent);
  }
  force_redraw = true;
}

static void PauseApp() {
  if (!active_app) return;

  if (active_app->_flags & APP_EV_MASK_AUDIO) {
    AnalogStop();
  } else {
    TickerStop();
  }
  MaskTick();
  if (active_app->OnPause) active_app->OnPause(active_app);
}

static void PopApp() {
  assert(active_app);
  PauseApp();
  if (active_app->OnStop) active_app->OnStop(active_app);
  active_app = active_app->_parent;
  if (active_app) ResumeApp();
}

static void PushApp(App * app) {
  assert(app);
  PauseApp();
  app->_parent = active_app;
  active_app = app;
  app->_flags = (active_app->OnStart ? app->OnStart(app) : 0);
  ResumeApp();
}

static void SwitchApp(App * app) {
  assert(app);
  assert(active_app);
  PauseApp();
  if (active_app->OnStop) active_app->OnStop(active_app);
  app->_parent = active_app->_parent;
  active_app = app;
  app->_flags = (active_app->OnStart ? app->OnStart(app) : 0);
  ResumeApp();
}

static void Tick(int16_t * audio_buffer) {
  if (tick_mask_count) return;

  assert(active_app);

  if (active_app->_flags & APP_EV_MASK_AUDIO) {
    assert(audio_buffer);
  } else {
    assert(!audio_buffer);
  }

  int16_t acc[3] = { 0, 0, 0 };
  TIMEOUT(2) {
    Error e = ImuRead(acc);
    assert(e == ERROR_NONE);
  }

  float x = acc[0];
  float y = acc[1];
  float z = acc[2];
  float norm = sqrtf(x*x + y*y + z*z);
  int16_t neck_tilt = asinf(y / norm) * 57.29577951308232f + 0.5f;
  int16_t screen_tilt = asinf(z / norm) * 57.29577951308232f + 0.5f;

  int8_t knob_turn_delta = 0;
  int8_t knob_press_delta = 0;
  if (active_app->_flags & APP_EV_MASK_KNOB) {
    uint8_t pos = KnobGetPosition();
    knob_turn_delta = pos - knob_prev_pos;
    knob_prev_pos = pos;

    bool pressed = KnobIsPressed();
    knob_press_delta = (pressed ? 1 : 0) - (knob_prev_pressed ? 1 : 0);
    knob_prev_pressed = pressed;
  }

  float vbat = AnalogGetVbat();
  // Very simplistic SoC model: assume that the voltage drops linearly with
  // SoC between 4.1V and 3.6V per cell.
  float soc = clamp(vbat - 7.2f, 0, 1) * 100;

  // To avoid toggling when we're on a borderline voltage, we implement some
  // hysteresis on the SoC: if the new SoC is within 1% of the previously
  // reported value, we leave it unchanged.
  uint8_t soc_percent;
  float dsoc = soc - prev_soc;
  if (abs(dsoc) < 1) {
    soc_percent = prev_soc;
  } else {
    soc_percent = (uint8_t) (soc + 0.5f);
    prev_soc = soc_percent;
  }

  GfxRect region = { 0, 0, DISPLAY_WIDTH, 16 };

  // Handle screen flipping. Screen should be flipped if the screen tilt angle
  // is > 25 (with +/-5 hysteresis).
  if (screen_flipped) {
    if (screen_tilt < 20) {
      screen_flipped = false;
      force_redraw = true;
    }
  } else {
    if (screen_tilt > 30) {
      screen_flipped = true;
      force_redraw = true;
    }
  }

  if (force_redraw) {
    DisplaySetRotation(screen_flipped ? 0x00 : 0xC0);
  }

  TitleBarDraw(&title_bar,
               &region,
               active_app->title ? active_app->title : "Untitled",
               soc_percent,
               force_redraw);

  region.y = 16;
  region.h = DISPLAY_HEIGHT - 16;

  memset(chain, 0, sizeof(chain));
  active_app->OnTick(active_app,
                     &region,
                     audio_buffer,
                     neck_tilt,
                     knob_turn_delta,
                     knob_press_delta,
                     force_redraw);
  ChainWrite(chain, sizeof(chain));
  force_redraw = false;
}

static void ProcessPendingCommand() {
  AppCommand command;
  if (ERROR_NONE != QueuePopFront(command_queue, &command)) return;

  switch (command.cmd) {
    case CMD_PUSH_APP   : PushApp((App *) command.arg1)  ; break;
    case CMD_POP_APP    : PopApp()                       ; break;
    case CMD_SWITCH_APP : SwitchApp((App *) command.arg1); break;
    case CMD_TICK       : Tick((int16_t *) command.arg1) ; break;
    case CMD_UNMASK_TICK: UnmaskTick();                  ; break;

    default:
      assert(command.cmd >= APP_CMD_MIN);
      if (active_app && active_app->OnCommand) {
        active_app->OnCommand(active_app, command);
      }
  }
}

static void AppTask(void * p) {
  App * main_app = (App *) p;

  PrngInit();
  TitleBarInit(&title_bar);
  AnalogInit();
  ChainInit();
  KnobInit();
  ImuInit();
  ImuOn();
  AudioProcInit();
  DisplayInit();
  GfxFill(&gfx_full_screen, RGB565(0, 0, 0));
  DisplaySetBacklight(0xFFFF);

  if (main_app) AppPostCommand(AppCommandPush(main_app));

  for (;;) {
    ProcessPendingCommand();
  }
}

void AppTaskInit(App * main_app) {
  command_queue = QueueCreate(4, sizeof(AppCommand));

  xTaskCreate(AppTask, "app", configMINIMAL_STACK_SIZE + 200, main_app, 2, &app_task);
  assert(app_task);
}

AppCommand AppCommandSwitch(App * app) {
  assert(app);
  return (AppCommand) {CMD_SWITCH_APP, (uint16_t) app};
}

AppCommand AppCommandPush(App * app) {
  assert(app);
  return (AppCommand) {CMD_PUSH_APP, (uint16_t) app};
}

AppCommand AppCommandPop() {
  return (AppCommand) {CMD_POP_APP};
}

AppCommand AppCommandNop() {
  return (AppCommand) {CMD_NOP};
}

void AppPostCommand(AppCommand cmd) {
  if (cmd.cmd == CMD_NOP) return;
  Error e = QueuePushBack(command_queue, &cmd);
  assert(e == ERROR_NONE);
}

void AppSetPixel(unsigned index, Rgb888 color) {
  assert(index < 10);
  index *= 3;
  chain[index++] = RGB888_R(color);
  chain[index++] = RGB888_G(color);
  chain[index++] = RGB888_B(color);
}
