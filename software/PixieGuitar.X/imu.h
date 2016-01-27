#ifndef IMU_H
#define	IMU_H

#include <stdint.h>

#include "error.h"

void ImuInit();
Error ImuOn();
Error ImuOff();
Error ImuRead(int16_t acc[3]);

#endif  // IMU_H

