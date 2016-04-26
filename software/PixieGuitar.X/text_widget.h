#ifndef TEXT_WIDGET_H
#define	TEXT_WIDGET_H

#include "widget.h"
#include "app.h"

typedef struct {
  // super-class
  Widget widget;

  // private
  char const * _str;
  AppCommand const * _cmd;
  WidgetState _last_drawn_state;
} TextWidget;

void TextWidgetInit(TextWidget * instance,
                    char const * str,
                    AppCommand const * cmd);

#endif  // TEXT_WIDGET_H

