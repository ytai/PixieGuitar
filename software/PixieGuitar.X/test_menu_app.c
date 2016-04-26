#include "test_menu_app.h"

#include <assert.h>
#include <string.h>

#include "display.h"

static uint16_t WidgetAppOnStart(void * instance) {
  assert(instance);
  return APP_EV_MASK_KNOB;
}
static void WidgetAppOnResume(void * instance) {
  assert(instance);

  WidgetApp * me = (WidgetApp *) instance;
  me->force_draw = true;
}

static void WidgetAppOnTick(void * instance,
                              GfxRect const * region,
                              int16_t * audio_samples,
                              int16_t acc[3],
                              int8_t knob_turn_delta,
                              int8_t knob_press_delta,
                              uint8_t soc_percent) {
  assert(instance);

  WidgetApp * me = (WidgetApp *) instance;
  me->widget->OnTick(&me->widget->instance,
                     region,
                     knob_turn_delta,
                     knob_press_delta,
                     me->force_draw,
                     WIDGET_ACTIVE);
  me->force_draw = false;
}

App * WidgetAppInit(WidgetApp * instance, Widget * widget) {
  memset(&instance->app, 0, sizeof(instance->app));
  instance->app.instance = instance;
  instance->app.OnStart = WidgetAppOnStart;
  instance->app.OnResume = WidgetAppOnResume;
  instance->app.OnTick = WidgetAppOnTick;

  instance->widget = widget;

  return &instance->app;
}

App * TestMenuAppInit(TestMenuApp * instance) {
  assert(instance);

  TextWidgetInit(&instance->text1, "First choice");
  TextWidgetInit(&instance->text2, "Second choice");
  TextWidgetInit(&instance->text3, "Third choice");
  TextWidgetInit(&instance->text4, "Fourth choice");

  instance->texts[0] = &instance->text1.widget;
  instance->texts[1] = &instance->text2.widget;
  instance->texts[2] = &instance->text3.widget;
  instance->texts[3] = &instance->text4.widget;
  VerticalWidgetListInit(&instance->list, instance->texts, 4);

  return WidgetAppInit(&instance->app, &instance->list.widget);
}

