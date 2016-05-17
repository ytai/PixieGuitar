#include "chart.h"

#include <assert.h>

void DrawBarGraph(GfxRect const * region,
                  uint16_t const * data,
                  size_t data_size,
                  int log2scale,
                  Rgb565 fg_color,
                  Rgb565 bg_color) {
  assert(region);
  assert(data || !data_size);

  if (data_size > region->w) data_size = region->w;

  for (size_t x = 0; x < data_size; ++x) {
    uint16_t y = data[x];
    if (log2scale > 0) {
      y <<= log2scale;
    } else if (log2scale < 0) {
      y >>= log2scale;
    }

    if (y > region->h) y = region->h;
    GfxDrawVerticalLine(region, x, 0, region->h - y, bg_color);
    GfxDrawVerticalLine(region, x, region->h - y, y, fg_color);
  }
}

void DrawVerticalVolumeBar(GfxRect const * region,
                           uint16_t vol,
                           Rgb565 fg_color,
                           Rgb565 bg_color) {
  assert(region);

  if (vol > region->h) vol = region->h;
  GfxFillRect(region, 0, 0, region->w, region->h - vol, bg_color);
  GfxFillRect(region, 0, region->h - vol, region->w, vol, fg_color);
}
