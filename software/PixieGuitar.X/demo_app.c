#include "demo_app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <dsp.h>

#include <FreeRTOS.h>
#include <task.h>

#include "analog.h"
#include "app.h"
#include "chain.h"
#include "display.h"
#include "gfx.h"
#include "imu.h"
#include "knob.h"
#include "sync.h"
#include "time.h"

static fractcomplex twiddle[ANALOG_BUFFER_LEN / 2] __attribute__((space(xmemory)));
static fractcomplex complex_buf[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));
static fractional window[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));

static uint8_t pixie_buf[3];

static uint8_t Tri(uint8_t x) {
  if (x < 43) return x * 6;
  if (x < 128) return 255;
  if (x < 171) return 1023 - x * 6;
  return 0;
}

static void PixieSetColor(uint8_t hue, uint8_t brightness) {
  pixie_buf[0] = ((uint16_t) Tri(hue +   0) * brightness) >> 8;
  pixie_buf[1] = ((uint16_t) Tri(hue +  85) * brightness) >> 8;
  pixie_buf[2] = ((uint16_t) Tri(hue + 170) * brightness) >> 8;
}

static uint16_t area[16][16];

typedef struct {
  uint8_t knob_turn;
  bool knob_pressed;
} DemoAppState;

static uint16_t DemoAppOnStart(void * instance) {
  DemoAppState * state = (DemoAppState *) instance;
  state->knob_turn = 0;
  state->knob_pressed = false;

  for (unsigned i = 0; i < 16; ++i)
    for (unsigned j = 0; j < 16; ++j)
      area[i][j] = RGB565(0xff, 0xff, 0xff);

  return APP_EV_MASK_ACC   |
         APP_EV_MASK_AUDIO |
         APP_EV_MASK_KNOB  |
         APP_EV_MASK_SOC;
}

static void DemoAppOnResume(void * instance) {
  GfxFillScreen(RGB565(0, 0, 0));
}

static void DemoAppOnTick(void * instance,
                         int16_t * audio_samples,
                         int16_t acc[3],
                         int8_t knob_turn_delta,
                         int8_t knob_press_delta,
                         uint8_t soc_percent) {
  DemoAppState * state = (DemoAppState *) instance;
  state->knob_turn += knob_turn_delta;
  state->knob_pressed = ((int) state->knob_pressed + knob_press_delta) > 0;

  char soc_str[15];
  snprintf(soc_str, sizeof(soc_str), "SoC=%3d%% P=%03d", soc_percent, state->knob_turn);

  VectorWindow(ANALOG_BUFFER_LEN, audio_samples, audio_samples, window);
  for (unsigned i = 0; i < ANALOG_BUFFER_LEN; ++i) {
    complex_buf[i].real = audio_samples[i] * state->knob_turn;
    complex_buf[i].imag = 0;
  }

  FFTComplexIP(ANALOG_LOG2_BUFFER_LEN, complex_buf, twiddle, COEFFS_IN_DATA);
  BitReverseComplex(ANALOG_LOG2_BUFFER_LEN, complex_buf);
  SquareMagnitudeCplx(ANALOG_BUFFER_LEN, complex_buf, audio_samples);

  uint16_t max_volume = 0;
  uint8_t max_index = 0;
  for (uint8_t i = 0; i < 64; ++i) {
    if (audio_samples[i + 4] > max_volume) {
      max_index = i;
      max_volume = audio_samples[i + 4];
    }
  }
  max_volume *= state->knob_turn;
  if (max_volume > 255) max_volume = 255;
  PixieSetColor(max_index << 2, max_volume);
  ChainWrite(pixie_buf, sizeof(pixie_buf));

  for (size_t y = 16; y < 160; ++y) {
    int16_t sample = audio_samples[y - 16];
//          uint8_t x = sample >> 9 ;
//          line[x] = RGB565(0xff, 0xff, 0x00);
//          DisplayCopyRect(0, y, 128, 1, line);
//          line[x] = 0;
    uint8_t x = sample * state->knob_turn;
    if (x > 128) x = 128;
    DisplayFillRect(0, y, x, 1, RGB565(0xff, 0xff, 0x00));
    DisplayFillRect(x, y, 128 - x, 1, RGB565(0x00, 0x00, 0x00));
  }

  GfxDrawString(10,
                1,
                soc_str,
                state->knob_pressed ? RGB565(0x00, 0xff, 0x00) : RGB565(0xff, 0xff, 0xff),
                RGB565(0, 0, 0));

  unsigned x = ((acc[0] ^ 0x4000) >> 11) & 0xF;
  unsigned y = ((acc[1] ^ 0x4000) >> 11) & 0xF;
  area[y][x] = RGB565(0xff, 0x00, 0x00);
  DisplayCopyRect(112, 0, 16, 16, area);
  area[y][x] = RGB565(0xff, 0xff, 0xff);
}

static DemoAppState demo_app_state;
static App demo_app;

App * DemoAppInit() {
  TwidFactorInit(ANALOG_LOG2_BUFFER_LEN, twiddle, 0);
  HanningInit(ANALOG_BUFFER_LEN, window);

  memset(&demo_app, 0, sizeof(demo_app));
  demo_app.instance = &demo_app_state;
  demo_app.OnStart = DemoAppOnStart;
  demo_app.OnResume = DemoAppOnResume;
  demo_app.OnTick = DemoAppOnTick;
  return &demo_app;
}

