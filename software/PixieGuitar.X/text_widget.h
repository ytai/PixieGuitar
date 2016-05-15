#ifndef TEXT_WIDGET_H
#define	TEXT_WIDGET_H

#include "widget.h"
#include "app.h"

typedef struct {
  // super-class
  Widget widget;

  // private
  char const * _str;
  uint16_t _color_bg_inactive;
  uint16_t _color_bg_active;
  uint16_t _color_fg_inactive;
  uint16_t _color_fg_active;
  AppCommand _cmd;
  WidgetState _last_drawn_state;
} TextWidget;

Widget * TextWidgetInit(TextWidget * instance,
                        char const * str,
                        unsigned width,
                        unsigned height,
                        uint16_t color_bg_inactive,
                        uint16_t color_bg_active,
                        uint16_t color_fg_inactive,
                        uint16_t color_fg_active,
                        AppCommand cmd);

#endif  // TEXT_WIDGET_H

