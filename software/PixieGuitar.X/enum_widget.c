#include "enum_widget.h"

#include <assert.h>
#include <stdio.h>

#include "util.h"

static bool EnumWidgetOnTick(Widget * instance,
                             GfxRect const * region,
                             int8_t knob_turn_delta,
                             int8_t knob_press_delta,
                             bool force_draw,
                             WidgetState state) {
  assert(instance);

  EnumWidget * me = (EnumWidget *) instance;
  bool highlighted = state != WIDGET_INACTIVE;
  bool active = state == WIDGET_ACTIVE;
  bool clicked = knob_press_delta < 0;

  uint16_t fg_color = active ? me->_color_fg_active : me->_color_fg_inactive;
  uint16_t bg_color = highlighted ? me->_color_bg_active : me->_color_bg_inactive;

  int nextval = me->val;

  switch (me->_mode) {
    case ENUM_WIDGET_CLICK:
      if (clicked) {
        nextval = mod(nextval + 1, me->_count);
      }
      break;

    case ENUM_WIDGET_TURN_WRAP:
      nextval = mod(nextval + knob_turn_delta, me->_count);
      break;

    case ENUM_WIDGET_TURN_NOWRAP:
      nextval = limit(nextval + knob_turn_delta, 0, me->_count - 1);
      break;

    default:
      assert(false);
  }

  bool value_changed = nextval != me->val;

  if (force_draw || state != me->_last_drawn_state || value_changed) {
    GfxFill(region, bg_color);

    GfxDrawString(region,
                  1,
                  region->h / 2 - 4,
                  me->_name,
                  fg_color,
                  bg_color);

    GfxDrawStringRightAlign(region,
                            region->w - 2,
                            region->h / 2 - 4,
                            me->_strings[nextval],
                            fg_color,
                            bg_color);

    me->_last_drawn_state = state;
  }

  if (value_changed) {
    me->val = nextval;
    AppPostCommand((AppCommand) { me->_cmd, (uint16_t) me->val });
  }
  return clicked;
}

Widget * EnumWidgetInit(EnumWidget * instance,
                        char const * name,
                        unsigned width,
                        unsigned height,
                        EnumWidgetMode mode,
                        char const * const * strings,
                        int count,
                        int defval,
                        uint16_t color_bg_inactive,
                        uint16_t color_bg_active,
                        uint16_t color_fg_inactive,
                        uint16_t color_fg_active,
                        uint16_t cmd) {
  assert(instance);
  assert(name);
  assert(defval < count);
  assert(defval >= 0);
  assert(strings);
  for (unsigned i = 0; i < count; ++i) assert(strings[i]);

  instance->widget.height = height;
  instance->widget.width = width;
  instance->widget.OnTick = EnumWidgetOnTick;

  instance->_name = name;
  instance->val = defval;
  instance->_mode = mode;
  instance->_strings = strings;
  instance->_count = count;
  instance->_color_bg_inactive = color_bg_inactive;
  instance->_color_bg_active = color_bg_active;
  instance->_color_fg_inactive = color_fg_inactive;
  instance->_color_fg_active = color_fg_active;
  instance->_cmd = cmd;

  return &instance->widget;

}

