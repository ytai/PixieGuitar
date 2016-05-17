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
#include "chart.h"

static uint16_t DemoAppOnStart(App * instance) {
  DemoApp * app = (DemoApp *) instance;
  app->knob_turn = 1;
  app->knob_pressed = false;

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

  uint16_t power;
  int16_t * bucket_pitch;
  int16_t * bucket_full;
  int16_t * bucket_octave;

  power = AudioProcPower(audio_samples);

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
  AppSetPixel(PrngGenerate8() % 10,
              Hsv2Rgb888((uint16_t) max_index * (1536 / BUCKET_COUNT),
                         0xFF,
                         max_volume));

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
    DrawBarGraph(&sub,
                 (uint16_t const *) bucket_pitch,
                 PITCH_COUNT,
                 0,
                 RGB565_GREEN,
                 RGB565_BLACK);
  }

  {
    GfxRect sub;
    GfxSubRegion(region, 0, 56, 24, 32, &sub);
    DrawBarGraph(&sub,
                 (uint16_t const *) bucket_full,
                 BUCKET_COUNT,
                 0,
                 RGB565_YELLOW,
                 RGB565_BLACK);
  }

  {
    GfxRect sub;
    GfxSubRegion(region, 32, 56, 24, 32, &sub);
    DrawBarGraph(&sub,
                 (uint16_t const *) bucket_octave,
                 BUCKET_COUNT,
                 0,
                 RGB565_RED,
                 RGB565_BLACK);
  }

  {
    GfxRect sub;
    GfxSubRegion(region, 64, 56, 10, 32, &sub);
    DrawVerticalVolumeBar(&sub,
                          power >> 5,
                          RGB565_BLUE,
                          RGB565_BLACK);
  }
}

App * DemoAppInit(DemoApp * instance) {
  assert(instance);

  memset(instance, 0, sizeof(DemoApp));
  instance->app.title = "Demo Application";
  instance->app.OnStart = DemoAppOnStart;
  instance->app.OnTick = DemoAppOnTick;
  return &instance->app;
}

