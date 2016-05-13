#include "rainbow_app.h"

#include <assert.h>

#include "chain.h"
#include "gfx.h"

static uint16_t RainbowAppOnStart(App * instance) {
  assert(instance);

  return APP_EV_MASK_KNOB;
}

static void RainbowAppOnResume(App * instance) {
  assert(instance);

  RainbowApp * app = (RainbowApp *) instance;
  app->frame_count = 0;
}

static uint8_t chain[10 * 3];

static void (RainbowAppOnTick) (App * instance,
                                GfxRect const * region,
                                int16_t * audio_samples,
                                int16_t tilt,
                                int8_t knob_turn_delta,
                                int8_t knob_press_delta,
                                uint8_t soc_percent,
                                bool force_redraw) {
  assert(instance);

  RainbowApp * app = (RainbowApp *) instance;

  if (force_redraw) {
    GfxFill(region, RGB565(0,0,0));

    GfxDrawString(region,
                  10,
                  10,
                  "Rainbow",
                  RGB565(0xff,0xff,0xff),
                  RGB565(0,0,0));
  }

  uint8_t * p = &chain[0];

  for (unsigned i = 0; i < 10; ++i) {
    uint16_t h = app->frame_count + i * 100;
    if (h >= 1536) h -= 1536;
    Rgb888 color = Hsv2Rgb888(h, 0xFF, 0x30);
    *p++ = RGB888_R(color);
    *p++ = RGB888_G(color);
    *p++ = RGB888_B(color);
  }
  ChainWrite(chain, sizeof(chain));

  app->frame_count += 6;
  if (app->frame_count >= 1536) app->frame_count -= 1536;

  if (knob_press_delta < 0) AppPostCommand(AppCommandPop());
}

App * RainbowAppInit(RainbowApp * instance) {
  assert(instance);

  App * app = &instance->app;
  app->title = "Rainbow";
  app->OnStart = RainbowAppOnStart;
  app->OnResume = RainbowAppOnResume;
  app->OnTick = RainbowAppOnTick;

  return app;
}

