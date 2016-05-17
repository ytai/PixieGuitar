#ifndef DEMO_APP_H
#define	DEMO_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "app.h"

typedef struct {
  App app;
  uint8_t knob_turn;
  bool knob_pressed;
} DemoApp;

App * DemoAppInit(DemoApp * instance);

#endif  // DEMO_APP_H

