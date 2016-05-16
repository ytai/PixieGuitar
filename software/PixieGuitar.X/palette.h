#ifndef PALETTE_H
#define	PALETTE_H

#include "color.h"

#define PALETTE_COUNT 7

extern char const * const palette_names[PALETTE_COUNT];

Rgb565 PaletteGetRgb565(unsigned palette, uint8_t color);
Rgb888 PaletteGetRgb888(unsigned palette, uint8_t color);

#endif  // PALETTE_H

