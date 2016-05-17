#ifndef CHART_H
#define	CHART_H

#include <stddef.h>

#include "gfx.h"

void DrawBarGraph(GfxRect const * region,
                  uint16_t const * data,
                  size_t data_size,
                  int log2scale,
                  Rgb565 fg_color,
                  Rgb565 bg_color);

void DrawVerticalVolumeBar(GfxRect const * region,
                           uint16_t vol,
                           Rgb565 fg_color,
                           Rgb565 bg_color);

#endif  // CHART_H

