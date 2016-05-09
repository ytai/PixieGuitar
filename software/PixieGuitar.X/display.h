// Interface with the ST7735 1.8" SPI LCD Module
// Code adapted from https://github.com/adafruit/Adafruit-ST7735-Library
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "color.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 160

void DisplayInit();

void DisplaySetBacklight(uint16_t level);

void DisplayFillRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     Rgb565 color);

void DisplayCopyRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     Rgb565 const * data);

void DisplayCopyRectFrag(uint8_t x,
                         uint8_t y,
                         uint8_t w,
                         uint8_t h,
                         uint8_t stride,
                         Rgb565 const * data);

void DisplaySetRotation(uint8_t m);

uint8_t DisplayGetRotation();

#endif  // DISPLAY_H
