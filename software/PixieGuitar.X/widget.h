#ifndef WIDGET_H
#define	WIDGET_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx.h"

typedef enum {
  WIDGET_INACTIVE,
  WIDGET_HIGHLIGHTED,
  WIDGET_ACTIVE
} WidgetState;

typedef struct _Widget {
  unsigned width;
  unsigned height;

  bool (*OnTick)(struct _Widget * instance,
                 GfxRect const * region,
                 int8_t knob_turn_delta,
                 int8_t knob_press_delta,
                 bool force_draw,
                 WidgetState state);
} Widget;

#endif  // WIDGET_H

