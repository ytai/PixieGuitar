#include "text_widget.h"
#include "display.h"

#include <assert.h>
#include <string.h>

static bool TextWidgetOnTick(void * instance,
                             GfxRect const * region,
                             int8_t knob_turn_delta,
                             int8_t knob_press_delta,
                             bool force_draw,
                             WidgetState state) {
  assert(instance);

  TextWidget * me = (TextWidget *) instance;
  bool highlighted = state != WIDGET_INACTIVE;
  bool active = state == WIDGET_ACTIVE;

  uint16_t fg_color = active ? me->_color_fg_active : me->_color_fg_inactive;
  uint16_t bg_color = highlighted ? me->_color_bg_active : me->_color_bg_inactive;

  if (force_draw || state != me->_last_drawn_state) {
    GfxFill(region, bg_color);
    GfxDrawString(region,
                  1,
                  me->widget.height / 2 - 4,
                  me->_str,
                  fg_color,
                  bg_color);
    me->_last_drawn_state = state;
  }

  bool released = knob_press_delta < 0;
  if (released && me->_cmd) {
    AppPostCommand(*me->_cmd);
  }
  return released;
}

void TextWidgetInit(TextWidget * instance,
                    char const * str,
                    unsigned width,
                    unsigned height,
                    uint16_t color_bg_inactive,
                    uint16_t color_bg_active,
                    uint16_t color_fg_inactive,
                    uint16_t color_fg_active,
                    AppCommand const * cmd) {
  assert(instance);
  assert(str);

  instance->widget.height = height;
  instance->widget.width = width;
  instance->widget.instance = instance;
  instance->widget.OnTick = TextWidgetOnTick;

  instance->_str = str;
  instance->_color_bg_inactive = color_bg_inactive;
  instance->_color_bg_active = color_bg_active;
  instance->_color_fg_inactive = color_fg_inactive;
  instance->_color_fg_active = color_fg_active;
  instance->_cmd = cmd;
}
