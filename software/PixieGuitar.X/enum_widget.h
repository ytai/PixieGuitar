#ifndef ENUM_WIDGET_H
#define	ENUM_WIDGET_H

#include "widget.h"
#include "app.h"

typedef enum {
  ENUM_WIDGET_CLICK,
  ENUM_WIDGET_TURN_WRAP,
  ENUM_WIDGET_TURN_NOWRAP
} EnumWidgetMode;

typedef struct {
  // super-class
  Widget widget;

  // public
  int val;

  // private
  char const * _name;
  EnumWidgetMode _mode;
  char const * const * _strings;
  int _count;
  uint16_t _color_bg_inactive;
  uint16_t _color_bg_active;
  uint16_t _color_fg_inactive;
  uint16_t _color_fg_active;
  uint16_t _cmd;
  WidgetState _last_drawn_state;
} EnumWidget;

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
                        uint16_t cmd);


#endif  // ENUM_WIDGET_H

