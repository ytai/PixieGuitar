#ifndef FIREWORKS_APP_H
#define	FIREWORKS_APP_H

#include <stdbool.h>

#include "app.h"
#include "enum_widget.h"
#include "number_widget.h"
#include "text_widget.h"
#include "vertical_widget_list.h"

#define FIREWORKS_APP_NUM_CHANNELS 4

typedef struct {
  uint8_t bucket;
  uint8_t s;
  uint8_t v;
} FireworksAppChannel;

typedef struct {
  App app;
  int16_t prev_tilt;
  FireworksAppChannel channels[FIREWORKS_APP_NUM_CHANNELS];
  EnumWidget mode_widget;
  TextWidget exit_widget;
  NumberWidget gain_widget;
  NumberWidget brightness_widget;
  NumberWidget value_fade_widget;
  NumberWidget sat_fade_widget;

  Widget * widgets[6];
  VerticalWidgetList widget_list;
  Widget * widget;
} FireworksApp;

App * FireworksAppInit(FireworksApp * instance);


#endif  // FIREWORKS_APP_H

