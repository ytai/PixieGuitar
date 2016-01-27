#include "clock.h"

#include <xc.h>

void ClockInit() {
  // Configure PLL prescaler, PLL postscaler, PLL divisor.
  // FRC clock is 7.37MHz. It is prescaled by a factor of 2, then multiplyed by
  // 76 to get 280MHz and then divide by 2 to get 140MHz, which is the system
  // clock.
  // Instruction clock would be half of that, or 70MHz (maximum).
  _PLLPRE = 0;   // N1=2
  _PLLDIV = 74;  // M=76
  _PLLPOST = 0;  // N2=2

  // Initiate Clock Switch to FRC oscillator with PLL (NOSC=0b001)
  __builtin_write_OSCCONH(0x01);
  __builtin_write_OSCCONL(OSCCON | 0x01);

  // Wait for Clock switch to occur
  while (_COSC != 0b001);
  // Wait for PLL to lock
  while (!_LOCK);
}

