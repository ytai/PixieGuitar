#ifndef GFX_H
#define	GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "color.h"

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
             Rgb565 const * data);

void GfxFill(GfxRect const * region,
             Rgb565 color);

void GfxFillRect(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h,
                 Rgb565 color);

void GfxDrawRect(GfxRect const * region,
                 int x,
                 int y,
                 uint8_t w,
                 uint8_t h,
                 Rgb565 color);
void GfxDrawVerticalLine(GfxRect const * region,
                         int x,
                         int y,
                         uint8_t length,
                         Rgb565 color);
void GfxDrawHorizontalLine(GfxRect const * region,
                         int x,
                         int y,
                         uint8_t length,
                         Rgb565 color);
//void GfxDrawFastLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color, uint8_t rotflag);
//void GfxDrawLine(int x0, int y0, int x1, int y1, uint16_t color);

void GfxDrawString(GfxRect const * region,
                   int x,
                   int y,
                   char const * str,
                   Rgb565 fg_color,
                   Rgb565 bg_color);

void GfxDrawStringRightAlign(GfxRect const * region,
                             int x,
                             int y,
                             char const * str,
                             Rgb565 fg_color,
                             Rgb565 bg_color);

void GfxDrawChar(GfxRect const * region,
                 int x,
                 int y,
                 char c,
                 Rgb565 fg_color,
                 Rgb565 bg_color);


#endif  // GFX_H

