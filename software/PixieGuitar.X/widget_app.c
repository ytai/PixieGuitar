#include "widget_app.h"

#include <assert.h>
#include <string.h>

static uint16_t WidgetAppOnStart(App * instance) {
  assert(instance);
  return APP_EV_MASK_KNOB;
}

static void WidgetAppOnTick(App * instance,
                            GfxRect const * region,
                            int16_t * audio_samples,
                            int16_t tilt,
                            int8_t knob_turn_delta,
                            int8_t knob_press_delta,
                            uint8_t soc_percent,
                            bool force_redraw) {
  assert(instance);

  WidgetApp * me = (WidgetApp *) instance;
  me->widget->OnTick(me->widget,
                     region,
                     knob_turn_delta,
                     knob_press_delta,
                     force_redraw,
                     WIDGET_ACTIVE);
}

App * WidgetAppInit(WidgetApp * instance,
                    char const * title,
                    void (*on_command)(App *, AppCommand),
                    Widget * widget) {
  memset(&instance->app, 0, sizeof(instance->app));
  instance->app.title = title;
  instance->app.OnStart = WidgetAppOnStart;
  instance->app.OnTick = WidgetAppOnTick;
  instance->app.OnCommand = on_command;

  instance->widget = widget;

  return &instance->app;
}

