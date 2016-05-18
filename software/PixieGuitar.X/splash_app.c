#include "splash_app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "app_cmd.h"
#include "display.h"
#include "power.h"
#include "util.h"

#define TIMEOUT_MS 20000

static uint16_t SplashAppOnStart(App * instance) {
  assert(instance);
  SplashApp * app = (SplashApp *) instance;
  app->locked = false;

  return APP_EV_MASK_KNOB;
}

static void SplashAppOnCommand(App * instance, AppCommand cmd) {
  assert(instance);
  assert(cmd.cmd == APP_CMD_START);

  SplashApp * app = (SplashApp *) instance;
  if (app->rock_widget.val == 1 &&
      app->and_widget.val == 2 &&
      app->roll_widget.val == 3) {
    AppPostCommand(AppCommandSwitch(app->switch_to));
  }
}

static void (SplashAppOnTick) (App * instance,
                               GfxRect const * region,
                               int16_t * audio_samples,
                               int16_t tilt,
                               int8_t knob_turn_delta,
                               int8_t knob_press_delta,
                               bool force_redraw) {
  assert(instance);
  SplashApp * app = (SplashApp *) instance;

  if (app->locked) {
    DisplaySetBacklight(0);
    PowerOff();
    return;
  }

  uint16_t ms_since_boot = xTaskGetTickCount() * portTICK_PERIOD_MS;
  if (ms_since_boot > TIMEOUT_MS) {
    app->locked = true;
    return;
  }


  GfxRect timer_region, widget_region;
  GfxSubRegion(region, 0, 0, region->w, 16, &timer_region);
  GfxSubRegion(region, 0, 16, region->w, region->h - 16, &widget_region);

  unsigned remaining_sec = (TIMEOUT_MS + 999 - ms_since_boot) / 1000;
  char timer_text[22];
  snprintf(timer_text, sizeof(timer_text), "The world ends in: %2u", remaining_sec);

  if (force_redraw) {
    GfxFill(&timer_region, RGB565_BLACK);
  }

  GfxDrawString(&timer_region,
                1,
                timer_region.h / 2 - 4,
                timer_text,
                RGB565_RED,
                RGB565_BLACK);

  app->widget->OnTick(app->widget,
                      &widget_region,
                      knob_turn_delta,
                      knob_press_delta,
                      force_redraw,
                      WIDGET_ACTIVE);
}

static char const * const words[] = { "Bla", "Rock", "and", "Roll!" };

App * SplashAppInit(SplashApp * instance, App * switch_to) {
  assert(instance);

  memset(instance, 0, sizeof(SplashApp));

  App * app = &instance->app;
  app->title = "Splash";
  app->OnStart = SplashAppOnStart;
  app->OnTick = SplashAppOnTick;
  app->OnCommand = SplashAppOnCommand;

  instance->switch_to = switch_to;

  instance->widgets[0] = EnumWidgetInit(&instance->rock_widget,
                                        "Ready",
                                        DISPLAY_WIDTH,
                                        16,
                                        ENUM_WIDGET_CLICK,
                                        words,
                                        ARRAY_LEN(words),
                                        0,
                                        RGB565_BLACK,
                                        RGB565_WHITE,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_BLACK,
                                        0);

  instance->widgets[1] = EnumWidgetInit(&instance->and_widget,
                                        "To",
                                        DISPLAY_WIDTH,
                                        16,
                                        ENUM_WIDGET_CLICK,
                                        words,
                                        ARRAY_LEN(words),
                                        0,
                                        RGB565_BLACK,
                                        RGB565_WHITE,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_BLACK,
                                        0);

  instance->widgets[2] = EnumWidgetInit(&instance->roll_widget,
                                        "Rumble?",
                                        DISPLAY_WIDTH,
                                        16,
                                        ENUM_WIDGET_CLICK,
                                        words,
                                        ARRAY_LEN(words),
                                        0,
                                        RGB565_BLACK,
                                        RGB565_WHITE,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_BLACK,
                                        0);


  instance->widgets[3] = TextWidgetInit(&instance->go_widget,
                                        "Go!",
                                        DISPLAY_WIDTH,
                                        16,
                                        RGB565_BLACK,
                                        RGB565_WHITE,
                                        RGB565_LIGHT_GRAY,
                                        RGB565_BLACK,
                                        (AppCommand) { APP_CMD_START });

  instance->widget = VerticalWidgetListInit(&instance->widget_list,
                                            instance->widgets,
                                            ARRAY_LEN(instance->widgets));

  return app;
}

