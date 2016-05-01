#include "test_menu_app.h"

#include <assert.h>
#include <string.h>

#include "display.h"
#include "demo_app.h"

static uint16_t WidgetAppOnStart(App * instance) {
  assert(instance);
  return APP_EV_MASK_KNOB;
}
static void WidgetAppOnResume(App * instance) {
  assert(instance);

  WidgetApp * me = (WidgetApp *) instance;
  me->force_draw = true;
}

static void WidgetAppOnTick(App * instance,
                            GfxRect const * region,
                            int16_t * audio_samples,
                            int16_t acc[3],
                            int8_t knob_turn_delta,
                            int8_t knob_press_delta,
                            uint8_t soc_percent) {
  assert(instance);

  WidgetApp * me = (WidgetApp *) instance;
  me->widget->OnTick(me->widget,
                     region,
                     knob_turn_delta,
                     knob_press_delta,
                     me->force_draw,
                     WIDGET_ACTIVE);
  me->force_draw = false;
}

App * WidgetAppInit(WidgetApp * instance, Widget * widget) {
  memset(&instance->app, 0, sizeof(instance->app));
  instance->app.OnStart = WidgetAppOnStart;
  instance->app.OnResume = WidgetAppOnResume;
  instance->app.OnTick = WidgetAppOnTick;

  instance->widget = widget;

  return &instance->app;
}

App * TestMenuAppInit(TestMenuApp * instance) {
  assert(instance);

  instance->launch_cmd = AppCommandPush(DemoAppInit());

#define WHITE RGB(0xFF, 0xFF, 0xFF)
#define GRAY  RGB(0x80, 0x80, 0x80)

  char const * const strings[] = {
    "First choice",
    "Second choice",
    "Launch App",
    "Fourth choice",
    "Fifth choice",
    "Sixth choice",
    "Seventh choice",
    "Eighth choice",
  };

  for (size_t i = 0; i < 8; ++i) {
    uint16_t h = 1535 * i / 8;
    TextWidgetInit(&instance->texts[i],
                   strings[i],
                   128,
                   16,
                   GfxHsv(h, 0x80, 0x30),
                   GfxHsv(h, 0xFF, 0x80),
                   GRAY,
                   WHITE,
                   i == 2 ? &instance->launch_cmd : NULL);
    instance->textp[i] = &instance->texts[i].widget;
  }

  VerticalWidgetListInit(&instance->list, instance->textp, 8);

  return WidgetAppInit(&instance->app, &instance->list.widget);
}

