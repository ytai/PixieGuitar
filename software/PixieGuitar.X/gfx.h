#ifndef GFX_H
#define	GFX_H

#include <stdint.h>

void GfxDrawPixel(uint8_t x, uint8_t y, uint16_t color);
void GfxFillScreen(uint16_t color);
void GfxDrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void GfxDrawVerticalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
void GfxDrawHorizontalLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
void GfxDrawFastLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color, uint8_t rotflag);
void GfxDrawLine(int x0, int y0, int x1, int y1, uint16_t color);
void GfxDrawString(uint8_t x, uint8_t y, char *c, uint16_t fg_color, uint16_t bg_color);
void GfxDrawChar(uint8_t x, uint8_t y, char c, uint16_t fg_color, uint16_t bg_color);



#endif  // GFX_H

