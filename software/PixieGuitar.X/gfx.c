#include "gfx.h"

#include <assert.h>

#include "display.h"
#include "font.h"

#define swap(a, b) { int t = a; a = b; b = t; }

GfxRect gfx_full_screen = { 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT };

bool GfxOverlaps(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h) {
  assert(region);

  return x + w > 0   &&
         x < region->w &&
         y + h > 0   &&
         y < region->h;
}

bool GfxSubRegion(GfxRect const * region,
                  int x,
                  int y,
                  uint8_t w,
                  uint8_t h,
                  GfxRect * result) {
  assert(region);
  assert(result);

  if (!GfxOverlaps(region, x, y, w, h)) {
    result->w = 0;
    result->h = 0;
    return false;
  }

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (x + w > region->w) {
    w = region->w - x;
  }

  if (y < 0) {
    h += y;
    y = 0;
  }
  if (y + h > region->h) {
    h = region->h - y;
  }

  result->x = x;
  result->y = y;
  result->w = w;
  result->h = h;

  return true;
}

void GfxCopy(GfxRect const * region,
             int x,
             int y,
             uint8_t w,
             uint8_t h,
             uint16_t const * data) {
  assert(region);
  // No-op if out of bounds.
  if (!GfxOverlaps(region, x, y, w, h)) return;

  // Clip top if necessary.
  if (y < 0) {
    h += y;
    data -= (y * w);
    y = 0;
  }

  // Clip bottom if necessary.
  if (h > region->h) {
    h = region->h - y;
  }

  if (x >= 0 && x + w <= region->w) {
    // No horizontal regionping - simple case.
    DisplayCopyRect(region->x + x, region->y + y, w, h, data);
  } else {
    uint8_t stride = w;

    // Clip left if necessary.
    if (x < 0) {
      w += x;
      data -= x;
      x = 0;
    }

    // Clip right if necessary.
    if (x + w > region->w) {
      w = region->w - x;
    }

    DisplayCopyRectFrag(region->x + x,
                        region->y + y,
                        w,
                        h,
                        stride,
                        data);
  }
}

void GfxFill(GfxRect const * region,
             uint16_t color) {
  assert(region);
  DisplayFillRect(region->x, region->y, region->w, region->h, color);
}

//void GfxDrawPixel(uint8_t x, uint8_t y, uint16_t color) {
//  DisplayFillRect(x, y, 1, 1, color);
//}
//
//void GfxFillScreen(uint16_t color)
//{
//  DisplayFillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
//}
//
//void GfxDrawRect(uint8_t x,
//                     uint8_t y,
//                     uint8_t w,
//                     uint8_t h,
//                     uint16_t color) {
//  GfxDrawHorizontalLine(x, y, w, color);
//  GfxDrawHorizontalLine(x, y+h-1, w, color);
//  GfxDrawVerticalLine(x, y, h, color);
//  GfxDrawVerticalLine(x+w-1, y, h, color);
//}
//
//void GfxDrawVerticalLine(uint8_t x,
//                             uint8_t y,
//                             uint8_t length,
//                             uint16_t color) {
//  if (x >= SCREEN_WIDTH) return;
//  if (y+length >= SCREEN_HEIGHT) length = SCREEN_HEIGHT-y-1;
//
//  GfxDrawFastLine(x,y,length,color,1);
//}
//
//void GfxDrawHorizontalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
//  if (y >= SCREEN_HEIGHT) return;
//  if (x+length >= SCREEN_WIDTH) length = SCREEN_WIDTH-x-1;
//
//  GfxDrawFastLine(x,y,length,color,0);
//}
//
//void GfxDrawFastLine(uint8_t x,
//                         uint8_t y,
//                         uint8_t length,
//                         uint16_t color,
//                         uint8_t rotflag) {
//  if (rotflag) {
//    DisplayFillRect(x, y, 1, length, color);
//  } else {
//    DisplayFillRect(x, y, length, 1, color);
//  }
//}
//
//int abs(int val)
//{
//  return (val > 0) ? val : -val;
//}
//
//// bresenham's algorithm - thx wikpedia
//void GfxDrawLine(int x0, int y0, int x1, int y1,
//uint16_t color) {
//  uint16_t steep = abs(y1 - y0) > abs(x1 - x0);
//  if (steep) {
//    swap(x0, y0);
//    swap(x1, y1);
//  }
//
//  if (x0 > x1) {
//    swap(x0, x1);
//    swap(y0, y1);
//  }
//
//  uint16_t dx, dy;
//  dx = x1 - x0;
//  dy = abs(y1 - y0);
//
//  int err = dx / 2;
//  int ystep;
//
//  if (y0 < y1) {
//    ystep = 1;
//  } else {
//    ystep = -1;}
//
//  for (; x0<=x1; x0++) {
//    if (steep) {
//      GfxDrawPixel(y0, x0, color);
//    } else {
//      GfxDrawPixel(x0, y0, color);
//    }
//    err -= dy;
//    if (err < 0) {
//      y0 += ystep;
//      err += dx;
//    }
//  }
//}

void GfxDrawString(GfxRect const * region,
                   int x,
                   int y,
                   char const * str,
                   uint16_t fg_color,
                   uint16_t bg_color) {
  while (*str) {
    GfxDrawChar(region, x, y, *str, fg_color, bg_color);
    x += 6;
    ++str;
  }
}

void GfxDrawChar(GfxRect const * region,
                 int x,
                 int y,
                 char c,
                 uint16_t fg_color,
                 uint16_t bg_color) {
  // Fast return on invalid characters.
  if (c < 0x20 || c >= 0x80) return;

  // Fast return on out-of-bounds.
  if (!GfxOverlaps(region, x, y, 5, 8)) return;


  uint16_t buffer[8][5];
  uint8_t i, j;

  uint8_t letter = c - 0x20;
  uint8_t const * line_ptr = &font[letter * 5];
  for (i = 0; i < 5; ++i) {
    uint8_t line = *line_ptr++;

    for (j = 0; j<8; j++) {
      buffer[j][i] = (line & 0x1) ? fg_color : bg_color;
      line >>= 1;
    }
  }
  GfxCopy(region, x, y, 5, 8, &buffer[0][0]);
}

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

uint16_t GfxHsv(uint16_t h, uint8_t s, uint8_t v) {
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

  return RGB(r, g, b);
}
