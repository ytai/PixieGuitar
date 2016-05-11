#ifndef PRNG_H
#define	PRNG_H

#include <stddef.h>
#include <stdint.h>

void PrngInit();
void PrngGenerate(void * buf, size_t size);

uint8_t PrngGenerate8();
uint16_t PrngGenerate16();
uint32_t PrngGenerate32();

#endif  // PRNG_H

