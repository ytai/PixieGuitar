#ifndef GFX_H
#define	GFX_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
} GfxRect;

extern GfxRect gfx_full_screen;

bool GfxOverlaps(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h);

bool GfxSubRegion(GfxRect const * region,
                  int x,
                  int y,
                  uint8_t w,
                  uint8_t h,
                  GfxRect * result);

void GfxCopy(GfxRect const * region,
             int x,
             int y,
             uint8_t w,
             uint8_t h,
             uint16_t const * data);

void GfxFill(GfxRect const * region,
             uint16_t color);

//void GfxDrawPixel(GfxRect const * region,
//                  uint8_t x,
//                  uint8_t y,
//                  uint16_t color);
//
//void GfxFillScreen(uint16_t color);
//void GfxDrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
//void GfxDrawVerticalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
//void GfxDrawHorizontalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
//void GfxDrawFastLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color, uint8_t rotflag);
//void GfxDrawLine(int x0, int y0, int x1, int y1, uint16_t color);

void GfxDrawString(GfxRect const * region,
                   int x,
                   int y,
                   char const * str,
                   uint16_t fg_color,
                   uint16_t bg_color);

void GfxDrawChar(GfxRect const * region,
                 int x,
                 int y,
                 char c,
                 uint16_t fg_color,
                 uint16_t bg_color);


#endif  // GFX_H

