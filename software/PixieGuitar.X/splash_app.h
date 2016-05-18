#ifndef SPLASH_APP_H
#define	SPLASH_APP_H

#include <stdbool.h>

#include "app.h"
#include "enum_widget.h"
#include "text_widget.h"
#include "vertical_widget_list.h"

typedef struct {
  App app;

  bool locked;
  App * switch_to;

  EnumWidget rock_widget;
  EnumWidget and_widget;
  EnumWidget roll_widget;
  TextWidget go_widget;

  Widget * widgets[4];
  VerticalWidgetList widget_list;
  Widget * widget;
} SplashApp;

App * SplashAppInit(SplashApp * instance, App * switch_to);


#endif  // SPLASH_APP_H

