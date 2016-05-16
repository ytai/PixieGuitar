#include "color.h"

#include <assert.h>
#include <stdbool.h>

static uint8_t Bump(uint16_t index) {
  uint8_t lsb = index & 0xFF;
  uint8_t msb = index >> 8;
  switch (msb) {
    case 0: case  6: return 0xFF;
    case 1: case  7: return 0xFF - lsb;
    case 2: case  8: return 0;
    case 3: case  9: return 0;
    case 4: case 10: return lsb;
    case 5: case 11: return 0xFF;
  }
  assert(false);
}

static inline uint8_t MulU8U8U8(uint8_t x, uint8_t y) {
  return ((uint16_t) x * (uint16_t) y) >> 8;
}

Rgb888 Hsv2Rgb888(uint16_t h, uint8_t s, uint8_t v) {
  assert(h < 1536);

  // Generate full-saturation, full-value color:
  uint8_t r = Bump(h);
  uint8_t g = Bump(h + 1024);
  uint8_t b = Bump(h + 512);

  // The s parameter specifies a linear mix with gray. Thus for any component y,
  // its value would be (pretending that values are 0..1):
  // v * (y * s + (1 - s)) = (v*s) * y + (v-v*s) = c * y + d
  uint8_t c = MulU8U8U8(s, v);
  uint8_t d = v - c;

  r = MulU8U8U8(r, c) + d;
  g = MulU8U8U8(g, c) + d;
  b = MulU8U8U8(b, c) + d;

  return RGB888(r, g, b);
}

Rgb888 Hsv2Rgb565(uint16_t h, uint8_t s, uint8_t v) {
  Rgb888 rgb888 = Hsv2Rgb888(h, s, v);
  return RGB888TO565(rgb888);
}

Rgb888 Rgb888Scale(Rgb888 c, uint8_t v) {
  return RGB888(MulU8U8U8(RGB888_R(c), v),
                MulU8U8U8(RGB888_G(c), v),
                MulU8U8U8(RGB888_B(c), v));
}

Rgb565 Rgb565Scale(Rgb565 c, uint8_t v) {
  return RGB565(MulU8U8U8(RGB565_R(c), v),
                MulU8U8U8(RGB565_G(c), v),
                MulU8U8U8(RGB565_B(c), v));
}
