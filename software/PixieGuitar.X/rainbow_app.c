#include "rainbow_app.h"

#include <assert.h>

#include "chain.h"
#include "gfx.h"
#include "display.h"
#include "text_widget.h"
#include "util.h"
#include "palette.h"
#include "base_macros.h"

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

  uint8_t c = app->frame_count;
  for (unsigned i = 0; i < 10; ++i) {
    c += app->diversity_widget.val;
    Rgb888 color = PaletteGetRgb888(app->palette_widget.val, c);
    color = Rgb888Scale(color, brightness);
    *p++ = RGB888_R(color);
    *p++ = RGB888_G(color);
    *p++ = RGB888_B(color);
  }
  ChainWrite(chain, sizeof(chain));

  app->frame_count += app->speed_widget.val;

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

  instance->widgets[1] = EnumWidgetInit(&instance->palette_widget,
                                        "Palette",
                                        DISPLAY_WIDTH,
                                        16,
                                        ENUM_WIDGET_TURN_WRAP,
                                        palette_names,
                                        ARRAY_LEN(palette_names),
                                        0,
                                        RGB565_BLACK,
                                        RGB565_DARK_GRAY,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_WHITE,
                                        0);

  instance->widgets[2] = NumberWidgetInit(&instance->brightness_widget,
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

  instance->widgets[3] = NumberWidgetInit(&instance->speed_widget,
                                          "Speed",
                                          DISPLAY_WIDTH,
                                          16,
                                          2,
                                          1,
                                          0,
                                          20,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[4] = NumberWidgetInit(&instance->diversity_widget,
                                          "Diversity",
                                          DISPLAY_WIDTH,
                                          16,
                                          20,
                                          5,
                                          0,
                                          50,
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

