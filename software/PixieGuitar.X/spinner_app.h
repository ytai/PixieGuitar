#ifndef SPINNER_APP_H
#define	SPINNER_APP_H

#include <stdint.h>

#include "app.h"
#include "number_widget.h"
#include "text_widget.h"
#include "vertical_widget_list.h"

typedef struct {
  App app;
  int16_t spinner1_count;
  int16_t spinner2_count;

  TextWidget exit_widget;
  NumberWidget brightness_widget;
  NumberWidget collision_widget;
  NumberWidget speed_widget;

  Widget * widgets[4];
  VerticalWidgetList widget_list;
  Widget * widget;
} SpinnerApp;

App * SpinnerAppInit(SpinnerApp * instance);


#endif  // SPINNER_APP_H

