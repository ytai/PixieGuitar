#ifndef ANALOG_H
#define	ANALOG_H

#include <stddef.h>
#include <stdint.h>

typedef void (*AnalogCallback)(int16_t * audio);

#define ANALOG_LOG2_BUFFER_LEN 10
#define ANALOG_BUFFER_LEN (1 << (ANALOG_LOG2_BUFFER_LEN))

void AnalogInit();
void AnalogStart(AnalogCallback callback);
void AnalogStop();
float AnalogGetVbat();

#endif	/* ANALOG_H */

