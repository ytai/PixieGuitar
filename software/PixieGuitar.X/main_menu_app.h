#ifndef MAIN_MENU_APP_H
#define	MAIN_MENU_APP_H

#include "text_widget.h"
#include "vertical_widget_list.h"
#include "widget_app.h"

typedef struct {
  WidgetApp app;

  VerticalWidgetList list;
  TextWidget texts[8];
  Widget * textp[8];
} MainMenuApp;

App * MainMenuAppInit(MainMenuApp * instance);

#endif  // MAIN_MENU_APP_H

