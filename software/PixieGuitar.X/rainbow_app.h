#ifndef RAINBOW_APP_H
#define	RAINBOW_APP_H

#include <stdbool.h>

#include "app.h"
#include "enum_widget.h"
#include "number_widget.h"
#include "text_widget.h"
#include "vertical_widget_list.h"

typedef struct {
  App app;
  uint8_t frame_count;
  int16_t prev_tilt;

  EnumWidget palette_widget;
  TextWidget exit_widget;
  NumberWidget brightness_widget;
  NumberWidget speed_widget;
  NumberWidget diversity_widget;

  Widget * widgets[5];
  VerticalWidgetList widget_list;
  Widget * widget;
} RainbowApp;

App * RainbowAppInit(RainbowApp * instance);

#endif  // RAINBOW_APP_H

