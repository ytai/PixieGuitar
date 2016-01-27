#ifndef ATOMIC_H
#define ATOMIC_H

static inline void atomic_u16_set(volatile uint16_t * p, uint16_t value) {
    asm volatile(
        "mov %1, [%0] \n"
        :
        : "r"(p), "r"(value));
}

static inline uint16_t atomic_u16_get(volatile uint16_t * p) {
  uint16_t result;
    asm volatile(
        "mov [%1], %0 \n"
        : "=r"(result)
        : "r"(p));
  return result;
}


#endif  // ATOMIC_H
