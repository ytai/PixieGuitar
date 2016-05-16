#include "palette.h"

#include <assert.h>
#include <stdbool.h>

char const * const palette_names[PALETTE_COUNT] = {
  "Rainbow",
  "Lime",
  "Pinky",
  "Happy",
};

static Rgb888 Rainbow(uint8_t color) {
  return Hsv2Rgb888((uint16_t) color * 6, 0xFF, 0xFF);
}

static Rgb888 Lime(uint8_t color) {
  return Hsv2Rgb888((uint16_t) color * 3 + 256, 0xFF, 0xFF);
}

static Rgb888 Pinky(uint8_t color) {
  uint16_t c = color;
  if (c < 0x40) return Hsv2Rgb888(c * 4 + 1280, 0xFF, 0xFF);
  if (c < 0x80) return Hsv2Rgb888(0, 511 - c * 4, 0xFF);
  return Hsv2Rgb888(1791 - c * 2, c * 2 - 255, 0xFF);
}

static Rgb888 Happy(uint8_t color) {
  uint16_t c = color;
  if (c < 0x80) return Hsv2Rgb888(c * 2, 0xFF, 0xFF);
  if (c < 0xC0) return Hsv2Rgb888(256, 767 - c * 4, 0xFF);
  return Hsv2Rgb888(1023 - c * 4, c * 4 - 768, 0xFF);
}

Rgb888 PaletteGetRgb888(unsigned palette, uint8_t color) {
  switch (palette) {
    case 0: return Rainbow(color);
    case 1: return Lime(color);
    case 2: return Pinky(color);
    case 3: return Happy(color);

    default: assert(false);
  }
}

Rgb565 PaletteGetRgb565(unsigned palette, uint8_t color) {
  return RGB888TO565(PaletteGetRgb888(palette, color));
}
