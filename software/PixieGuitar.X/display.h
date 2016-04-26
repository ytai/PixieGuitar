// Interface with the ST7735 1.8" SPI LCD Module
// Code adapted from https://github.com/adafruit/Adafruit-ST7735-Library
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 160

#define RGB565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

void DisplayInit();

void DisplaySetBacklight(uint16_t level);

void DisplayFillRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     uint16_t color);

void DisplayCopyRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     uint16_t const * data);

void DisplayCopyRectFrag(uint8_t x,
                         uint8_t y,
                         uint8_t w,
                         uint8_t h,
                         uint8_t stride,
                         uint16_t const * data);

void DisplaySetRotation(uint8_t m);

uint8_t DisplayGetRotation();

#endif  // DISPLAY_H
