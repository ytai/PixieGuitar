#ifndef TEST_MENU_APP_H
#define	TEST_MENU_APP_H

#include "app.h"
#include "vertical_widget_list.h"
#include "text_widget.h"

typedef struct {
  App app;
  Widget * widget;
  bool force_draw;
} WidgetApp;

App * WidgetAppInit(WidgetApp * instance, char const * title, Widget * widget);

typedef struct {
  WidgetApp app;

  AppCommand launch_cmd;
  VerticalWidgetList list;
  TextWidget texts[8];
  Widget * textp[8];
} TestMenuApp;

App * TestMenuAppInit(TestMenuApp * instance);

#endif  // TEST_MENU_APP_H

