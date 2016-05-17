#include "spinner_app.h"

#include <assert.h>
#include <string.h>

#include "audio_proc.h"
#include "chart.h"
#include "display.h"
#include "util.h"

static uint16_t SpinnerAppOnStart(App * instance) {
  assert(instance);

  return APP_EV_MASK_KNOB | APP_EV_MASK_AUDIO;
}

static void SpinnerAppOnResume(App * instance) {
  assert(instance);

  SpinnerApp * app = (SpinnerApp *) instance;
  app->spinner1_count = 0;
  app->spinner2_count = 0;
}

static void (SpinnerAppOnTick) (App * instance,
                                GfxRect const * region,
                                int16_t * audio_samples,
                                int16_t tilt,
                                int8_t knob_turn_delta,
                                int8_t knob_press_delta,
                                bool force_redraw) {
  assert(instance);

  SpinnerApp * app = (SpinnerApp *) instance;

  uint16_t speed1 = AudioProcPower(audio_samples);
  if (speed1 < 128) {
    speed1 = 0;
  } else {
    speed1 -= 128;
  }
  if (speed1 > 1000) speed1 = 1000;

  int16_t speed2 = app->speed_widget.val * 100;
  uint8_t brightness = app->brightness_widget.val * 255 / 100;
  uint8_t collision = app->collision_widget.val * 255 / 100;

  unsigned index1 = app->spinner1_count / 1000;
  unsigned index2 = app->spinner2_count / 1000;
  if (index1 == index2) {
    AppSetPixel(index1, RGB888(collision, collision, collision));
  } else {
    AppSetPixel(index1, RGB888(brightness, 0x00, 0x00));
    AppSetPixel(index2, RGB888(0x00, 0x00, brightness));
  }

  app->spinner1_count = mod(app->spinner1_count + speed1, 10000);
  app->spinner2_count = mod(app->spinner2_count - speed2, 10000);

  GfxRect volume_region, widget_region;

  GfxSubRegion(region, 0, 0, region->w, 4, &volume_region);
  GfxSubRegion(region, 0, 4, region->w, region->h - 4, &widget_region);

  DrawHorizontalVolumeBar(&volume_region,
                          speed1 / 8,
                          RGB565(0x80, 0x80, 0xC0),
                          RGB565_BLACK);

  app->widget->OnTick(app->widget,
                      &widget_region,
                      knob_turn_delta,
                      knob_press_delta,
                      force_redraw,
                      WIDGET_ACTIVE);
}

App * SpinnerAppInit(SpinnerApp * instance) {
  assert(instance);

  memset(instance, 0, sizeof(SpinnerApp));

  App * app = &instance->app;
  app->title = "Spinner";
  app->OnStart = SpinnerAppOnStart;
  app->OnResume = SpinnerAppOnResume;
  app->OnTick = SpinnerAppOnTick;

  instance->widgets[0] = NumberWidgetInit(&instance->brightness_widget,
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

  instance->widgets[1] = NumberWidgetInit(&instance->speed_widget,
                                          "Speed",
                                          DISPLAY_WIDTH,
                                          16,
                                          2,
                                          1,
                                          1,
                                          10,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[2] = NumberWidgetInit(&instance->collision_widget,
                                          "Collision",
                                          DISPLAY_WIDTH,
                                          16,
                                          40,
                                          5,
                                          0,
                                          100,
                                          RGB565_BLACK,
                                          RGB565_DARK_GRAY,
                                          RGB565_LIGHT_GRAY,
                                          RGB565_WHITE,
                                          0);

  instance->widgets[3] = TextWidgetInit(&instance->exit_widget,
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

