#include "gfx.h"

#include <assert.h>

#include "display.h"
#include "font.h"
#include "color.h"

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

  result->x = region->x + x;
  result->y = region->y + y;
  result->w = w;
  result->h = h;

  return true;
}

void GfxCopy(GfxRect const * region,
             int x,
             int y,
             uint8_t w,
             uint8_t h,
             Rgb565 const * data) {
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
             Rgb565 color) {
  assert(region);
  DisplayFillRect(region->x, region->y, region->w, region->h, color);
}

void GfxFillRect(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h,
                 Rgb565 color) {
  GfxRect rect;
  GfxSubRegion(region, x, y, w, h, &rect);
  if (!rect.w || !rect.h) return;
  GfxFill(&rect, color);
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

void GfxDrawRect(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h,
                 Rgb565 color) {
  assert(region);
  assert(w);
  assert(h);

  GfxDrawHorizontalLine(region, x, y, w, color);
  GfxDrawHorizontalLine(region, x, y+h-1, w, color);
  GfxDrawVerticalLine(region, x, y+1, h-2, color);
  GfxDrawVerticalLine(region, x+w-1, y+1, h-2, color);
}

void GfxDrawVerticalLine(GfxRect const * region,
                         int x,
                         int y,
                         uint8_t length,
                         Rgb565 color) {
  assert(region);

  if (length == 0) return;
  if (x < 0 || x >= region->w) return;
  if (y + length <= 0 || y >= region->h) return;

  if (y < 0) {
    length += y;
    y = 0;
  }

  if (y + length > region->h) {
    length = region->h - y;
  }

  DisplayFillRect(region->x + x, region->y + y, 1, length, color);
}

void GfxDrawHorizontalLine(GfxRect const * region,
                         int x,
                         int y,
                         uint8_t length,
                         Rgb565 color) {
  assert(region);

  if (length == 0) return;
  if (y < 0 || y >= region->h) return;
  if (x + length <= 0 || x >= region->w) return;

  if (x < 0) {
    length += x;
    x = 0;
  }

  if (x + length > region->w) {
    length = region->w - x;
  }

  DisplayFillRect(region->x + x, region->y + y, length, 1, color);
}

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
                   Rgb565 fg_color,
                   Rgb565 bg_color) {
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
                 Rgb565 fg_color,
                 Rgb565 bg_color) {
  // Fast return on invalid characters.
  if (c < 0x20 || c >= 0x80) return;

  // Fast return on out-of-bounds.
  if (!GfxOverlaps(region, x, y, 5, 8)) return;


  Rgb565 buffer[8][5];
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
