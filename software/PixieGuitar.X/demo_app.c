#include "demo_app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "analog.h"
#include "audio_proc.h"
#include "app.h"
#include "chain.h"
#include "display.h"
#include "gfx.h"
#include "imu.h"
#include "knob.h"
#include "prng.h"
#include "sync.h"
#include "time.h"

static uint8_t pixie_buf[30];

static void PixieClear() {
  memset(pixie_buf, 0, sizeof(pixie_buf));
}

static void PixieSetColor(unsigned index, uint8_t hue, uint8_t brightness) {
  index *= 3;
  Rgb888 c = Hsv2Rgb888((uint16_t) hue * 64, 0xFF, brightness);
  pixie_buf[index + 0] = RGB888_R(c);
  pixie_buf[index + 1] = RGB888_G(c);
  pixie_buf[index + 2] = RGB888_B(c);
}

static uint16_t area[16][16];

typedef struct {
  App app;
  uint8_t knob_turn;
  bool knob_pressed;
} DemoApp;

static uint16_t DemoAppOnStart(App * instance) {
  DemoApp * app = (DemoApp *) instance;
  app->knob_turn = 1;
  app->knob_pressed = false;

  for (unsigned i = 0; i < 16; ++i)
    for (unsigned j = 0; j < 16; ++j)
      area[i][j] = RGB565(0xff, 0xff, 0xff);

  return APP_EV_MASK_AUDIO |
         APP_EV_MASK_KNOB;
}

static void DemoAppOnTick(App * instance,
                          GfxRect const * region,
                          int16_t * audio_samples,
                          int16_t tilt,
                          int8_t knob_turn_delta,
                          int8_t knob_press_delta,
                          bool force_redraw) {
  DemoApp * app = (DemoApp *) instance;
  app->knob_turn += knob_turn_delta;
  app->knob_pressed = ((int) app->knob_pressed + knob_press_delta) > 0;

  if (knob_press_delta < 0) AppPostCommand(AppCommandPop());

  int16_t * bucket_pitch;
  int16_t * bucket_full;
  int16_t * bucket_octave;

  AudioProcAnalyzePitch(audio_samples,
                        app->knob_turn,
                        &bucket_pitch,
                        &bucket_full,
                        &bucket_octave);

  uint16_t max_volume = 0;
  uint8_t max_index = 0;
  for (uint8_t i = 0; i < BUCKET_COUNT; ++i) {
    if (bucket_octave[i] > max_volume) {
      max_index = i;
      max_volume = bucket_octave[i];
    }
  }

  if (max_volume > 255) max_volume = 255;
  PixieClear();
  PixieSetColor(PrngGenerate8() % 10, max_index, max_volume);
  ChainWrite(pixie_buf, sizeof(pixie_buf));

  if (force_redraw) {
    GfxFill(region, RGB565(0, 0, 0));
  }

  char info_str[16];
  snprintf(info_str, sizeof(info_str), "Tilt=%+03d P=%03d", tilt, app->knob_turn);
  GfxDrawString(region,
                10,
                1,
                info_str,
                app->knob_pressed ? RGB565(0x00, 0xff, 0x00) : RGB565(0xff, 0xff, 0xff),
                RGB565(0, 0, 0));

  {
    GfxRect sub;
    GfxSubRegion(region, 0, 16, region->w, 32, &sub);
    for (unsigned i = 0; i < PITCH_COUNT; ++i) {
      int16_t y = bucket_pitch[i];
      if (y < 0) y = 0;
      if (y > 32) y = 32;
      GfxDrawVerticalLine(&sub, i, 0, 32 - y, RGB565(0x00, 0x00, 0x00));
      GfxDrawVerticalLine(&sub, i, 32 - y, y, RGB565(0x00, 0xff, 0x00));
    }
  }

  {
    GfxRect sub;
    GfxSubRegion(region, 0, 48, region->w, 32, &sub);
    for (unsigned i = 0; i < BUCKET_COUNT; ++i) {
      int16_t y = bucket_full[i];
      if (y < 0) y = 0;
      if (y > 32) y = 32;
      GfxDrawVerticalLine(&sub, i, 0, 32 - y, RGB565(0x00, 0x00, 0x00));
      GfxDrawVerticalLine(&sub, i, 32 - y, y, RGB565(0xff, 0xff, 0x00));
    }
  }

  {
    GfxRect sub;
    GfxSubRegion(region, 0, 80, region->w, 32, &sub);
    for (unsigned i = 0; i < BUCKET_COUNT; ++i) {
      int16_t y = bucket_octave[i];
      if (y < 0) y = 0;
      if (y > 32) y = 32;
      GfxDrawVerticalLine(&sub, i, 0, 32 - y, RGB565(0x00, 0x00, 0x00));
      GfxDrawVerticalLine(&sub, i, 32 - y, y, RGB565(0xff, 0x00, 0x00));
    }
  }
}

static DemoApp demo_app;

App * DemoAppInit() {
  memset(&demo_app, 0, sizeof(demo_app));
  demo_app.app.title = "Demo Application";
  demo_app.app.OnStart = DemoAppOnStart;
  demo_app.app.OnTick = DemoAppOnTick;
  return &demo_app.app;
}

