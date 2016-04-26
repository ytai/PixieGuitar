#ifndef TEXT_WIDGET_H
#define	TEXT_WIDGET_H

#include "widget.h"

typedef struct {
  // super-class
  Widget widget;

  // private
  char const * _str;
  WidgetState _last_drawn_state;
} TextWidget;

void TextWidgetInit(TextWidget * instance, char const * str);

#endif  // TEXT_WIDGET_H

