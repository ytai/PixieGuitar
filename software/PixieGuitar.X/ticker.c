#include "ticker.h"

#include <xc.h>
#include <FreeRTOSConfig.h>

#include "analog.h"
#include "portmacro.h"

static TickerCallback ticker_callback;

void TickerStart(TickerCallback callback) {
  ticker_callback = callback;
  // We're aiming to fake the rate of the analog module, which generates events
  // at a frequency of (20kHz / ANALOG_BUFFER_LEN).
  // Therefore, assuming our system clock is running at 70MHz, and we use a 1:64
  // prescaler, our period becomes:
  // (70MHz * ANALOG_BUFFER_LEN / 64 / 20kHz) = 3500 * (ANALOG_BUFFER_LEN / 64).
  PR2 = 3500u * (ANALOG_BUFFER_LEN / 64u);
  TMR2 = 0;
  _T2IP = configKERNEL_INTERRUPT_PRIORITY;
  _T2IE = 1;
  T2CON = 0x8020;
}

void TickerStop() {
  _T2IE = 0;
  T2CON = 0;
  _T2IF = 0;
  ticker_callback = NULL;
}

void __attribute__((interrupt, no_auto_psv, naked)) _T2Interrupt() {
  portSAVE_CONTEXT();
  _T2IF = 0;
  ticker_callback();
  portRESTORE_CONTEXT();
}

