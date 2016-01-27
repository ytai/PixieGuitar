#include "console.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOSConfig.h>
#include <pps.h>

#include "sync.h"
#include "portmacro.h"

static void * tx_event;
static void * rx_event;

void ConsoleInit() {
  U1BRG = 151;
  U1MODE = 0x8008;
  U1STA = 0x0400;
  _U1TXIP = configKERNEL_INTERRUPT_PRIORITY;
  _U1RXIP = configKERNEL_INTERRUPT_PRIORITY;

  PPSInput(IN_FN_PPS_U1RX, IN_PIN_PPS_RP40);
  PPSOutput(OUT_FN_PPS_U1TX, OUT_PIN_PPS_RP39);

  tx_event = SemaphoreCreateBinary();
  rx_event = SemaphoreCreateBinary();
}

Error WaitCanTx() {
  U1STAbits.UTXISEL0 = 0;  // Interrupt whenever TX buffer is not full.
  while (U1STAbits.UTXBF) {
    if (_IPL == 0) {
      _U1TXIE = 1;
      Error e = SemaphoreTake(tx_event);
      if (e != ERROR_NONE) return e;
    }
  }
  return ERROR_NONE;
}

Error WaitTxComplete() {
  U1STAbits.UTXISEL0 = 1;  // Interrupt whenever TX is complete.
  while (!U1STAbits.TRMT) {
    if (_IPL == 0) {
      _U1TXIE = 1;
      Error e = SemaphoreTake(tx_event);
      if (e != ERROR_NONE) return e;
    }
  }
  return ERROR_NONE;
}

int __attribute__((__section__(".libc.write")))
write(int handle, void *buffer, unsigned int len) {
  char * p = (char *) buffer;
  unsigned total = len;
  switch (handle) {
    case 1:
    case 2:
      while (len--) {
        if (ERROR_NONE != WaitCanTx()) return total;
        U1TXREG = (*p++);
      }
      WaitTxComplete();
      return total;
  }
  return -1;
}

Error WaitReadAvailable() {
  while (!U1STAbits.URXDA) {
    if (_IPL == 0) {
      _U1RXIE = 1;
      Error e = SemaphoreTake(rx_event);
      if (e != ERROR_NONE) return e;
    }
  }
  return ERROR_NONE;
}

int __attribute__((__section__(".libc.read")))
read(int handle, void *buffer, unsigned int len) {
  uint8_t * p = (uint8_t *) buffer;
  unsigned total = 0;
  switch (handle) {
    case 0:
      // Read however many characters are available, up to len.
      while (total == 0 || (U1STAbits.URXDA && total < len)) {
        Error e = WaitReadAvailable();
        if (ERROR_NONE != e) return total;
        bool err = U1STAbits.FERR || U1STAbits.PERR;
        uint8_t c = U1RXREG;
        if (U1STAbits.OERR) U1STAbits.OERR = 0;
        if (err) continue;
        if (c == '\r') c = '\n';
        (*p++) = c;
        ++total;
      }
      return total;
  }
  return -1;
}

void __attribute__((interrupt, no_auto_psv, naked)) _U1TXInterrupt() {
  portSAVE_CONTEXT();
  _U1TXIF = 0;
  _U1TXIE = 0;
  SemaphoreGiveFromISR(tx_event);
  portRESTORE_CONTEXT();
}

void __attribute__((interrupt, no_auto_psv, naked)) _U1RXInterrupt() {
  portSAVE_CONTEXT();
  _U1RXIF = 0;
  _U1RXIE = 0;
  SemaphoreGiveFromISR(rx_event);
  portRESTORE_CONTEXT();
}

