#include "rainbow_app.h"

#include <assert.h>

#include "chain.h"
#include "gfx.h"
#include "display.h"
#include "text_widget.h"
#include "util.h"

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
                                bool force_redraw) {
  assert(instance);

  RainbowApp * app = (RainbowApp *) instance;

  uint8_t * p = &chain[0];

  uint8_t brightness = app->brightness_widget.val * 255 / 100;

  uint16_t h = app->frame_count;
  for (unsigned i = 0; i < 10; ++i) {
    h += app->diversity_widget.val * 3;
    if (h >= 1536) h -= 1536;
    Rgb888 color = Hsv2Rgb888(h, 0xFF, brightness);
    *p++ = RGB888_R(color);
    *p++ = RGB888_G(color);
    *p++ = RGB888_B(color);
  }
  ChainWrite(chain, sizeof(chain));

  app->frame_count += app->speed_widget.val;
  if (app->frame_count >= 1536) app->frame_count -= 1536;

  app->widget->OnTick(app->widget,
                      region,
                      knob_turn_delta,
                      knob_press_delta,
                      force_redraw,
                      WIDGET_ACTIVE);
}

App * RainbowAppInit(RainbowApp * instance) {
  assert(instance);

  App * app = &instance->app;
  app->title = "Rainbow";
  app->OnStart = RainbowAppOnStart;
  app->OnResume = RainbowAppOnResume;
  app->OnTick = RainbowAppOnTick;

  instance->widgets[0] = TextWidgetInit(&instance->exit_widget,
                                        "Exit",
                                        DISPLAY_WIDTH,
                                        16,
                                        RGB565_BLACK,
                                        RGB565_DARK_GRAY,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_WHITE,
                                        AppCommandPop());

  instance->widgets[1] = NumberWidgetInit(&instance->brightness_widget,
                                          "Brightness",
                                          DISPLAY_WIDTH,
                                          16,
                                          20,
                                          5,
                                          0,
                                          100,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[2] = NumberWidgetInit(&instance->speed_widget,
                                          "Speed",
                                          DISPLAY_WIDTH,
                                          16,
                                          20,
                                          5,
                                          0,
                                          100,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[3] = NumberWidgetInit(&instance->diversity_widget,
                                          "Diversity",
                                          DISPLAY_WIDTH,
                                          16,
                                          20,
                                          5,
                                          0,
                                          100,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widget = VerticalWidgetListInit(&instance->widget_list,
                                            instance->widgets,
                                            ARRAY_LEN(instance->widgets));

  return app;
}

