#include "imu.h"

#include "i2c.h"

#define ADDRESS    0x1D

#define REG_STATUS 0x00
#define REG_OUT    0x01
#define REG_CTL1   0x2A

void ImuInit() {
  I2cInit();
}

Error ImuOn() {
  uint8_t const req[] = { REG_CTL1, 0x01 };
  return I2cWriteRead(ADDRESS, req, sizeof(req), NULL, 0);
}

Error ImuOff() {
  uint8_t const req[] = { REG_CTL1, 0x00 };
  return I2cWriteRead(ADDRESS, req, sizeof(req), NULL, 0);
}

Error ImuRead(int16_t acc[3]) {
  uint8_t const req[] = { REG_OUT };
  uint8_t res[6];
  Error e = I2cWriteRead(ADDRESS, req, sizeof(req), res, sizeof(res));
  if (e != ERROR_NONE) return e;
  acc[0] = (res[0] << 8 ) | res[1];
  acc[1] = (res[2] << 8 ) | res[3];
  acc[2] = (res[4] << 8 ) | res[5];
  return ERROR_NONE;
}
