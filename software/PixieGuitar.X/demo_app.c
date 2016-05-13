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
#include "prng.h"
#include "sync.h"
#include "time.h"

static fractcomplex twiddle[ANALOG_BUFFER_LEN / 2] __attribute__((space(xmemory)));
static fractcomplex complex_buf[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));
static fractional window[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));

static uint8_t pixie_buf[30];

static void PixieClear() {
  memset(pixie_buf, 0, sizeof(pixie_buf));
}

static void PixieSetColor(unsigned index, uint8_t hue, uint8_t brightness) {
  index *= 3;
  Rgb888 c = Hsv2Rgb888((uint16_t) hue * 6, 0xFF, brightness);
  pixie_buf[index + 0] = RGB888_R(c);
  pixie_buf[index + 1] = RGB888_G(c);
  pixie_buf[index + 2] = RGB888_B(c);
}

static uint16_t area[16][16];

typedef struct {
  App app;
  uint8_t knob_turn;
  bool knob_pressed;
  bool force_draw;
} DemoApp;

static void DemoAppOnResume(App * instance) {
  DemoApp * app = (DemoApp *) instance;
  app->force_draw = true;
}

static uint16_t DemoAppOnStart(App * instance) {
  DemoApp * app = (DemoApp *) instance;
  app->knob_turn = 1;
  app->knob_pressed = false;

  for (unsigned i = 0; i < 16; ++i)
    for (unsigned j = 0; j < 16; ++j)
      area[i][j] = RGB565(0xff, 0xff, 0xff);

  return APP_EV_MASK_ACC   |
         APP_EV_MASK_AUDIO |
         APP_EV_MASK_KNOB;
}

static void DemoAppOnTick(App * instance,
                          GfxRect const * region,
                          int16_t * audio_samples,
                          int16_t acc[3],
                          int8_t knob_turn_delta,
                          int8_t knob_press_delta,
                          uint8_t soc_percent) {
  DemoApp * app = (DemoApp *) instance;
  app->knob_turn += knob_turn_delta;
  app->knob_pressed = ((int) app->knob_pressed + knob_press_delta) > 0;

  if (knob_press_delta < 0) AppPostCommand(AppCommandPop());

  char soc_str[15];
  snprintf(soc_str, sizeof(soc_str), "SoC=%3d%% P=%03d", soc_percent, app->knob_turn);

  VectorWindow(ANALOG_BUFFER_LEN, audio_samples, audio_samples, window);
  for (unsigned i = 0; i < ANALOG_BUFFER_LEN; ++i) {
    complex_buf[i].real = audio_samples[i] * app->knob_turn;
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
  max_volume *= app->knob_turn;
  if (max_volume > 255) max_volume = 255;
  PixieClear();
  PixieSetColor(PrngGenerate8() % 10, max_index << 2, max_volume);
  ChainWrite(pixie_buf, sizeof(pixie_buf));

  int16_t const * p = audio_samples;
  for (int y = 16; y < region->h; ++y) {
    int16_t sample = *p++;
//          uint8_t x = sample >> 9 ;
//          line[x] = RGB565(0xff, 0xff, 0x00);
//          DisplayCopyRect(0, y, 128, 1, line);
//          line[x] = 0;
    uint8_t x = sample * app->knob_turn;
    if (x > 128) x = 128;
    GfxDrawHorizontalLine(region, 0, y, x, RGB565(0xff, 0xff, 0x00));
    GfxDrawHorizontalLine(region, x, y, 128 - x, RGB565(0x00, 0x00, 0x00));
  }

  if (app->force_draw) {
    GfxFillRect(region,
                0,
                0,
                region->w - 16,
                16,
                RGB565(0, 0, 0));
  }
  GfxDrawString(region,
                10,
                1,
                soc_str,
                app->knob_pressed ? RGB565(0x00, 0xff, 0x00) : RGB565(0xff, 0xff, 0xff),
                RGB565(0, 0, 0));

  unsigned x = ((acc[0] ^ 0x4000) >> 11) & 0xF;
  unsigned y = ((acc[1] ^ 0x4000) >> 11) & 0xF;
  area[y][x] = RGB565(0xff, 0x00, 0x00);
  GfxCopy(region, region->w - 16, 0, 16, 16, &area[0][0]);
  area[y][x] = RGB565(0xff, 0xff, 0xff);

  app->force_draw = false;
}

static DemoApp demo_app;

App * DemoAppInit() {
  TwidFactorInit(ANALOG_LOG2_BUFFER_LEN, twiddle, 0);
  HanningInit(ANALOG_BUFFER_LEN, window);

  memset(&demo_app, 0, sizeof(demo_app));
  demo_app.app.title = "Demo Application";
  demo_app.app.OnResume = DemoAppOnResume;
  demo_app.app.OnStart = DemoAppOnStart;
  demo_app.app.OnTick = DemoAppOnTick;
  return &demo_app.app;
}

