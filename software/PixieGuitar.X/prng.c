#include "prng.h"

#include <xc.h>

#define PUSH(b) *((volatile uint8_t *) &CRCDATL) = b;

void PrngInit() {
  CRCCON1 = 0x8000;   // Enable the module.
  CRCCON2 = 0x071F;   // 8-bit data word, 32-bit CRC.
  CRCXORL = 0x1DB7;   // Configure polynomial.
  CRCXORH = 0x04C1;
  CRCWDATL = 0xFFFF;  // Seed.
  CRCWDATH = 0xFFFF;
  // Push 4 bytes to start.
  PUSH(0);
  PUSH(0);
  PUSH(0);
  PUSH(0);
  _CRCGO = 1;        // Run.
}

void PrngGenerate(void * buf, size_t size) {
  // Use 32-bit for most of the buffer.
  uint32_t * p32 = (uint32_t *) buf;
  size_t s4 = size / 4;
  while (s4--) {
    *p32++ = PrngGenerate32();
  }
  size &= 3;

  // Fill the rest using 8-bit.
  uint8_t * p8 = (uint8_t *) p32;
  while (size--) {
    *p8++ = PrngGenerate8();
  }
}

uint8_t PrngGenerate8() {
  while (!_CRCIF);
  _CRCGO = 0;
  _CRCIF = 0;
  uint8_t result = CRCWDATH >> 8;
  PUSH(0);
  _CRCGO = 1;
  return result;
}

uint16_t PrngGenerate16() {
  while (!_CRCIF);
  _CRCGO = 0;
  _CRCIF = 0;
  uint16_t result = CRCWDATH;
  PUSH(0);
  PUSH(0);
  _CRCGO = 1;
  return result;
}

uint32_t PrngGenerate32() {
  while (!_CRCIF);
  _CRCGO = 0;
  _CRCIF = 0;
  uint32_t result = ((uint32_t) CRCWDATH << 16) | CRCWDATL;
  PUSH(0);
  PUSH(0);
  PUSH(0);
  PUSH(0);
  _CRCGO = 1;
  return result;
}

