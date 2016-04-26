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

App * WidgetAppInit(WidgetApp * instance, Widget * widget);

typedef struct {
  WidgetApp app;

  AppCommand launch_cmd;
  VerticalWidgetList list;
  TextWidget text1;
  TextWidget text2;
  TextWidget text3;
  TextWidget text4;
  Widget * texts[4];
} TestMenuApp;

App * TestMenuAppInit(TestMenuApp * instance);

#endif  // TEST_MENU_APP_H

