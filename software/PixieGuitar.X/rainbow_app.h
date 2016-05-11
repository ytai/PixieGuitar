#ifndef RAINBOW_APP_H
#define	RAINBOW_APP_H

#include <stdbool.h>

#include "app.h"

typedef struct {
  App app;
  bool need_redraw;
  unsigned frame_count;
} RainbowApp;

App * RainbowAppInit(RainbowApp * instance);

#endif  // RAINBOW_APP_H

