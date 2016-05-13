#ifndef WIDGET_APP_H
#define	WIDGET_APP_H

#include "app.h"
#include "widget.h"

typedef struct {
  App app;
  Widget * widget;
  bool force_draw;
} WidgetApp;

App * WidgetAppInit(WidgetApp * instance,
                    char const * title,
                    void (*on_command)(App *, AppCommand),
                    Widget * widget);


#endif  // WIDGET_APP_H

