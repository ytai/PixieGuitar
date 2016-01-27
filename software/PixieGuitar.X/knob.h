#ifndef KNOB_H
#define	KNOB_H

#include <stdbool.h>
#include <stdint.h>

void KnobInit();
uint8_t KnobGetPosition();
bool KnobIsPressed();

#endif  // KNOB_H

