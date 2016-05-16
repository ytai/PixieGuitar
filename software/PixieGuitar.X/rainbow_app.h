#ifndef RAINBOW_APP_H
#define	RAINBOW_APP_H

#include <stdbool.h>

#include "app.h"
#include "number_widget.h"
#include "text_widget.h"
#include "vertical_widget_list.h"

typedef struct {
  App app;
  unsigned frame_count;

  TextWidget exit_widget;
  NumberWidget brightness_widget;
  NumberWidget speed_widget;
  NumberWidget diversity_widget;

  Widget * widgets[4];
  VerticalWidgetList widget_list;
  Widget * widget;
} RainbowApp;

App * RainbowAppInit(RainbowApp * instance);

#endif  // RAINBOW_APP_H

