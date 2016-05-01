#include "title_bar.h"
#include <assert.h>
#include <stddef.h>

// TODO:temp
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>

#include "display.h"

#define BG_COLOR RGB(0x40, 0x40, 0x40)
#define FG_COLOR RGB(0xFF, 0xFF, 0xFF)
#define BLACK    RGB(0x00, 0x00, 0x00)

void TitleBarInit(TitleBar * instance) {
  assert(instance);

  instance->_last_soc = 0xFF;
  instance->_last_title = NULL;
}

void TitleBarDraw(TitleBar * instance,
                  GfxRect const * region,
                  char const * title,
                  uint8_t soc_percent) {
  assert(instance);
  assert(region);
  assert(title);

  // No need to use strcmp. Worst-case, we'll redraw needlessly.
  bool redraw_title = instance->_last_title != title;
  bool redraw_soc = soc_percent != instance->_last_soc;

  if (redraw_title) {
    GfxRect title_region;
    GfxSubRegion(region,
                 0,
                 0,
                 region->w - 28,
                 region->h,
                 &title_region);

    GfxFill(&title_region, BG_COLOR);
    GfxDrawString(&title_region,
                  1,
                  region->h / 2 - 4,
                  title,
                  FG_COLOR,
                  BG_COLOR);
    instance->_last_title = title;
  }

  if (redraw_soc) {
    GfxRect soc_region;
    GfxSubRegion(region,
                 region->w - 28,
                 0,
                 28,
                 region->h,
                 &soc_region);
    GfxFill(&soc_region, BG_COLOR);

    // Draw battery outline.
    GfxDrawRect(&soc_region, 4, soc_region.h / 2 - 6, 22, 12, FG_COLOR);

    // Draw battery "+" terminal.
    GfxFillRect(&soc_region, 2, soc_region.h / 2 - 2, 2, 4, FG_COLOR);

    // Fill battery.
    char soc_text[4];
    if (soc_percent < 100) {
      snprintf(soc_text, sizeof(soc_text), "%2u%%", soc_percent);
    } else {
      strncpy(soc_text, "MAX", sizeof(soc_text));
    }

    uint16_t soc_color = GfxHsv(512u * soc_percent / 100, 0xFF, 0x80);
    uint8_t soc_width = soc_percent / 5;

    // Divide inside of battery into two regions: the "empty" on the left and
    // the "full" on the right.
    GfxRect soc_empty;
    GfxSubRegion(&soc_region,
                 5,
                 soc_region.h / 2 - 5,
                 20 - soc_width,
                 10,
                 &soc_empty);

    GfxRect soc_full;
    GfxSubRegion(&soc_region,
                 25 - soc_width,
                 soc_region.h / 2 - 5,
                 soc_width,
                 10,
                 &soc_full);

    // Print text once into each region, each time with a different BG color.
    GfxFill(&soc_empty, BLACK);
    GfxDrawString(&soc_empty, 1, 1, soc_text, FG_COLOR, BLACK);
    GfxFill(&soc_full, soc_color);
    GfxDrawString(&soc_full, (int) soc_width - 19, 1, soc_text, FG_COLOR, soc_color);

    instance->_last_soc = soc_percent;
  }
}
