#include "gfx.h"

#include "display.h"
#include "font.h"

#define swap(a, b) { int t = a; a = b; b = t; }

void GfxDrawPixel(uint8_t x, uint8_t y, uint16_t color) {
  DisplayFillRect(x, y, 1, 1, color);
}

void GfxFillScreen(uint16_t color)
{
  DisplayFillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
}

void GfxDrawRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     uint16_t color) {
  GfxDrawHorizontalLine(x, y, w, color);
  GfxDrawHorizontalLine(x, y+h-1, w, color);
  GfxDrawVerticalLine(x, y, h, color);
  GfxDrawVerticalLine(x+w-1, y, h, color);
}

void GfxDrawVerticalLine(uint8_t x,
                             uint8_t y,
                             uint8_t length,
                             uint16_t color) {
  if (x >= SCREEN_WIDTH) return;
  if (y+length >= SCREEN_HEIGHT) length = SCREEN_HEIGHT-y-1;

  GfxDrawFastLine(x,y,length,color,1);
}

void GfxDrawHorizontalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
  if (y >= SCREEN_HEIGHT) return;
  if (x+length >= SCREEN_WIDTH) length = SCREEN_WIDTH-x-1;

  GfxDrawFastLine(x,y,length,color,0);
}

void GfxDrawFastLine(uint8_t x,
                         uint8_t y,
                         uint8_t length,
                         uint16_t color,
                         uint8_t rotflag) {
  if (rotflag) {
    DisplayFillRect(x, y, 1, length, color);
  } else {
    DisplayFillRect(x, y, length, 1, color);
  }
}

int abs(int val)
{
  return (val > 0) ? val : -val;
}

// bresenham's algorithm - thx wikpedia
void GfxDrawLine(int x0, int y0, int x1, int y1,
uint16_t color) {
  uint16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int err = dx / 2;
  int ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      GfxDrawPixel(y0, x0, color);
    } else {
      GfxDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void GfxDrawString(uint8_t x, uint8_t y, char *c, uint16_t fg_color, uint16_t bg_color) {
  while (c[0] != 0) {
    GfxDrawChar(x, y, c[0], fg_color, bg_color);
    x += 6;
    c++;
    if (x + 5 >= SCREEN_WIDTH) {
      y += 10;
      x = 0;
    }
  }
}

void GfxDrawChar(uint8_t x,
                 uint8_t y,
                 char c,
                 uint16_t fg_color,
                 uint16_t bg_color) {
  uint16_t buffer[8][5];

  uint8_t i, j;

  uint8_t letter = c < 0x52 ? c - 0x20 : c - 0x52;
  for (i = 0; i < 5; ++i) {
    uint8_t line = c < 0x52 ? Alpha1[letter*5+i] : Alpha2[letter*5+i];

    for (j = 0; j<8; j++) {
      buffer[j][i] = (line & 0x1) ? fg_color : bg_color;
      line >>= 1;
    }
  }
  DisplayCopyRect(x, y, 5, 8, &buffer[0][0]);
}

