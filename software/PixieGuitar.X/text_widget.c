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

  uint16_t fg_color = active ? RGB565(0xFF, 0xFF, 0xFF) : RGB565(0x80, 0x80, 0x80);
  uint16_t bg_color = highlighted ? RGB565(0x00, 0x00, 0x80) : RGB565(0, 0, 0);

  if (force_draw || state != me->_last_drawn_state) {
    GfxFill(region, bg_color);
    GfxDrawString(region, 1, 1, me->_str, fg_color, bg_color);
    me->_last_drawn_state = state;
  }
  return knob_press_delta < 0;
}

void TextWidgetInit(TextWidget * instance, char const * str) {
  assert(instance);
  assert(str);

  size_t len = strlen(str);

  instance->widget.height = 10;
  instance->widget.width = len * 6 + 1;
  instance->widget.instance = instance;
  instance->widget.OnTick = TextWidgetOnTick;

  instance->_str = str;
}
