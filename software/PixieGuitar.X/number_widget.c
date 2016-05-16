#include "number_widget.h"

#include <assert.h>
#include <stdio.h>

static bool NumberWidgetOnTick(Widget * instance,
                               GfxRect const * region,
                               int8_t knob_turn_delta,
                               int8_t knob_press_delta,
                               bool force_draw,
                               WidgetState state) {
  assert(instance);

  NumberWidget * me = (NumberWidget *) instance;
  bool highlighted = state != WIDGET_INACTIVE;
  bool active = state == WIDGET_ACTIVE;

  uint16_t fg_color = active ? me->_color_fg_active : me->_color_fg_inactive;
  uint16_t bg_color = highlighted ? me->_color_bg_active : me->_color_bg_inactive;

  int nextval = me->val +  me->_step * knob_turn_delta;
  if (nextval > me->_maxval) nextval = me->_maxval;
  if (nextval < me->_minval) nextval = me->_minval;

  bool value_changed = nextval != me->val;

  if (force_draw || state != me->_last_drawn_state || value_changed) {
    GfxFill(region, bg_color);
    GfxRect name_region, value_region;
    GfxSubRegion(region, 1, 1, region->w - 28, region->h - 2, &name_region);
    GfxSubRegion(region, region->w - 25, 1, 23, region->h - 2, &value_region);

    GfxDrawString(&name_region,
                  0,
                  name_region.h / 2 - 4,
                  me->_str,
                  fg_color,
                  bg_color);

    char value_str[5];
    snprintf(value_str, sizeof(value_str), "%4d", nextval);
    GfxDrawString(&value_region,
                  0,
                  value_region.h / 2 - 4,
                  value_str,
                  fg_color,
                  bg_color);

    me->_last_drawn_state = state;
  }

  bool released = knob_press_delta < 0;
  if (value_changed) {
    me->val = nextval;
    AppPostCommand((AppCommand) { me->_cmd, (uint16_t) me->val });
  }
  return released;
}

Widget * NumberWidgetInit(NumberWidget * instance,
                          char const * str,
                          unsigned width,
                          unsigned height,
                          int defval,
                          int step,
                          int minval,
                          int maxval,
                          uint16_t color_bg_inactive,
                          uint16_t color_bg_active,
                          uint16_t color_fg_inactive,
                          uint16_t color_fg_active,
                          uint16_t cmd) {
  assert(instance);
  assert(str);
  assert(defval >= minval && defval <= maxval);

  instance->widget.height = height;
  instance->widget.width = width;
  instance->widget.OnTick = NumberWidgetOnTick;

  instance->_str = str;
  instance->val = defval;
  instance->_step = step;
  instance->_minval = minval;
  instance->_maxval = maxval;
  instance->_color_bg_inactive = color_bg_inactive;
  instance->_color_bg_active = color_bg_active;
  instance->_color_fg_inactive = color_fg_inactive;
  instance->_color_fg_active = color_fg_active;
  instance->_cmd = cmd;

  return &instance->widget;
}
