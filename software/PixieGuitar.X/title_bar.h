#ifndef TITLE_BAR_H
#define	TITLE_BAR_H

#include <stdint.h>

#include "gfx.h"

typedef struct {
  char const * _last_title;
  uint8_t _last_soc;
} TitleBar;

void TitleBarInit(TitleBar * instance);

void TitleBarDraw(TitleBar * instance,
                  GfxRect const * region,
                  char const * title,
                  uint8_t soc_percent);


#endif  // TITLE_BAR_H

