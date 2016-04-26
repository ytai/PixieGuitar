#include "app.h"

#include <assert.h>

#include <FreeRTOS.h>
#include <task.h>

#include "sync.h"
#include "display.h"
#include "gfx.h"
#include "time.h"
#include "imu.h"
#include "analog.h"
#include "knob.h"
#include "ticker.h"

#define CMD_TICK        0x0000
#define CMD_PUSH_APP    0x0001
#define CMD_POP_APP     0x0002
#define CMD_SWITCH_APP  0x0003
#define CMD_UNMASK_TICK 0x0004

static App * active_app = NULL;
static uint8_t knob_prev_pos;
static bool knob_prev_pressed;

static TaskHandle_t app_task;
static void * command_queue;
static unsigned tick_mask_count = 0;

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
  NONBLOCKING() AppPostCommand(&cmd);
}

static void UnmaskTick() {
  assert(tick_mask_count);
  --tick_mask_count;
}

static void ResumeApp() {
  GfxFill(&gfx_full_screen, RGB565(0, 0, 0));
  if (active_app->OnResume) active_app->OnResume(active_app->instance);

  if (active_app->_flags & APP_EV_MASK_ACC) {
    ImuOn();
  }
  if (active_app->_flags & APP_EV_MASK_AUDIO) {
    AnalogStart(AudioEvent);
  } else {
    TickerStart(TickerEvent);
  }
}

static void PauseApp() {
  if (!active_app) return;
  if (active_app->_flags & APP_EV_MASK_ACC) {
    ImuOff();
  }
  if (active_app->_flags & APP_EV_MASK_AUDIO) {
    AnalogStop();
  } else {
    TickerStop();
  }
  MaskTick();
  if (active_app->OnPause) active_app->OnPause(active_app->instance);
}

static void PopApp() {
  assert(active_app);
  PauseApp();
  if (active_app->OnStop) active_app->OnStop(active_app->instance);
  active_app = active_app->_parent;
  if (active_app) ResumeApp();
}

static void PushApp(App * app) {
  assert(app);
  PauseApp();
  app->_parent = active_app;
  active_app = app;
  app->_flags = (active_app->OnStart ? app->OnStart(app->instance) : 0);
  ResumeApp();
}

static void SwitchApp(App * app) {
  assert(app);
  assert(active_app);
  PauseApp();
  if (active_app->OnStop) active_app->OnStop(active_app->instance);
  app->_parent = active_app->_parent;
  active_app = app;
  app->_flags = (active_app->OnStart ? app->OnStart(app->instance) : 0);
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
  if (active_app->_flags & APP_EV_MASK_ACC) {
    ImuRead(acc);
  }

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

  uint8_t soc_percent;
  if (active_app->_flags & APP_EV_MASK_SOC) {
    float vbat = AnalogGetVbat();
    // Very simplistic SoC model: assume that the voltage drops linearly with
    // SoC between 4.1V and 3.6V per cell.
    if (vbat < 7.2f) soc_percent = 0;
    else if (vbat > 8.2f) soc_percent = 100;
    else soc_percent = (uint8_t) ((vbat - 7.2f) * 100);
  }

  active_app->OnTick(active_app->instance,
                     &gfx_full_screen,
                     audio_buffer,
                     acc,
                     knob_turn_delta,
                     knob_press_delta,
                     soc_percent);
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
        active_app->OnCommand(active_app->instance, &command);
      }
  }
}

static void AppTask(void * p) {
  App * main_app = (App *) p;
  if (main_app) AppPush(p);

  AnalogInit();
  KnobInit();
  ImuInit();
  DisplayInit();
  GfxFill(&gfx_full_screen, RGB565(0, 0, 0));
  DisplaySetBacklight(0xFFFF);

  for (;;) {
    ProcessPendingCommand();
  }
}

void AppTaskInit(App * main_app) {
  command_queue = QueueCreate(4, sizeof(AppCommand));

  xTaskCreate(AppTask, "app", configMINIMAL_STACK_SIZE + 200, main_app, 2, &app_task);
  assert(app_task);
}

void AppSwitch(App * app) {
  assert(app);
  AppCommand cmd = { CMD_SWITCH_APP, (uint16_t) app };
  AppPostCommand(&cmd);
}

void AppPush(App * app) {
  assert(app);
  AppCommand cmd = { CMD_PUSH_APP, (uint16_t) app };
  AppPostCommand(&cmd);
}

void AppPop() {
  AppCommand cmd = { CMD_POP_APP };
  AppPostCommand(&cmd);
}

void AppPostCommand(AppCommand const * cmd) {
  Error e = QueuePushBack(command_queue, cmd);
  assert(e == ERROR_NONE);
}
