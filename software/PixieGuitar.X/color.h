#ifndef COLOR_H
#define	COLOR_H

#include <stdint.h>

typedef uint16_t Rgb565;
typedef uint32_t Rgb888;

#define RGB565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define RGB565_R(c) (((c) >> 8) & 0xF8)
#define RGB565_G(c) (((c) >> 3) & 0xFC)
#define RGB565_B(c) (((c) << 3) & 0xF8)

#define RGB888(r,g,b) ((((uint32_t) (r) & 0xFF) << 16) | \
                       (((uint32_t) (g) & 0xFF) <<  8) | \
                       (((uint32_t) (b) & 0xFF) <<  0))
#define RGB888_R(c) (((c) >> 16) & 0xFF)
#define RGB888_G(c) (((c) >>  8) & 0xFF)
#define RGB888_B(c) (((c) >>  0) & 0xFF)

#define RGB565TO888(c) RGB888(RGB565_R(c), RGB565_G(c), RGB565_B(c))
#define RGB888TO565(c) RGB565(RGB888_R(c), RGB888_G(c), RGB888_B(c))

Rgb888 Hsv2Rgb888(uint16_t h, uint8_t s, uint8_t v);
Rgb888 Hsv2Rgb565(uint16_t h, uint8_t s, uint8_t v);

#define RGB565_BLACK      RGB565(0x00, 0x00, 0x00)
#define RGB565_WHITE      RGB565(0xFF, 0xFF, 0xFF)
#define RGB565_GRAY       RGB565(0x80, 0x80, 0x80)
#define RGB565_LIGHT_GRAY RGB565(0xC0, 0xC0, 0xC0)
#define RGB565_DARK_GRAY  RGB565(0x40, 0x40, 0x40)
#define RGB565_RED        RGB565(0xFF, 0x00, 0x00)
#define RGB565_GREEN      RGB565(0x00, 0xFF, 0x00)
#define RGB565_BLUE       RGB565(0x00, 0x00, 0xFF)
#define RGB565_YELLOW     RGB565(0xFF, 0xFF, 0x00)
#define RGB565_PURPLE     RGB565(0xFF, 0x00, 0xFF)
#define RGB565_CYAN       RGB565(0x00, 0xFF, 0xFF)

#endif  // COLOR_H

