#ifndef NUMBER_WIDGET_H
#define	NUMBER_WIDGET_H

#include "widget.h"
#include "app.h"

typedef struct {
  // super-class
  Widget widget;

  // public
  int val;

  // private
  char const * _str;
  int _step;
  int _minval;
  int _maxval;
  uint16_t _color_bg_inactive;
  uint16_t _color_bg_active;
  uint16_t _color_fg_inactive;
  uint16_t _color_fg_active;
  uint16_t _cmd;
  WidgetState _last_drawn_state;
} NumberWidget;

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
                          uint16_t cmd);


#endif  // NUMBER_WIDGET_H

