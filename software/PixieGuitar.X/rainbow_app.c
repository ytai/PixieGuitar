#include "rainbow_app.h"

#include <assert.h>
#include <string.h>

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
  app->prev_tilt = 0x7fff;
}

static void (RainbowAppOnTick) (App * instance,
                                GfxRect const * region,
                                int16_t * audio_samples,
                                int16_t tilt,
                                int8_t knob_turn_delta,
                                int8_t knob_press_delta,
                                bool force_redraw) {
  assert(instance);

  RainbowApp * app = (RainbowApp *) instance;

  uint8_t brightness = app->brightness_widget.val * 255 / 100;
  int16_t delta_tilt = app->prev_tilt == 0x7FFF ? 0 : tilt - app->prev_tilt;

  if (delta_tilt > 10) {
    // Lightning strike!
    for (unsigned i = 0; i < 10; ++i) {
      AppSetPixel(i, RGB888(brightness, brightness, brightness));
    }
  } else {
    uint8_t c = app->frame_count;
    for (unsigned i = 0; i < 10; ++i) {
      c += app->diversity_widget.val;
      AppSetPixel(i, Rgb888Scale(PaletteGetRgb888(app->palette_widget.val, c),
                                 brightness));
    }
  }

  app->frame_count += app->speed_widget.val;
  app->prev_tilt = tilt;

  app->widget->OnTick(app->widget,
                      region,
                      knob_turn_delta,
                      knob_press_delta,
                      force_redraw,
                      WIDGET_ACTIVE);
}

App * RainbowAppInit(RainbowApp * instance) {
  assert(instance);

  memset(instance, 0, sizeof(RainbowApp));

  App * app = &instance->app;
  app->title = "Rainbow";
  app->OnStart = RainbowAppOnStart;
  app->OnResume = RainbowAppOnResume;
  app->OnTick = RainbowAppOnTick;

  instance->widgets[0] = EnumWidgetInit(&instance->palette_widget,
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
                                          2,
                                          1,
                                          0,
                                          20,
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
                                          50,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[4] = TextWidgetInit(&instance->exit_widget,
                                        "Exit",
                                        DISPLAY_WIDTH,
                                        16,
                                        RGB565_BLACK,
                                        RGB565_DARK_GRAY,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_WHITE,
                                        AppCommandPop());

  instance->widget = VerticalWidgetListInit(&instance->widget_list,
                                            instance->widgets,
                                            ARRAY_LEN(instance->widgets));

  return app;
}

