#include "chain.h"

#include <assert.h>
#include <stdint.h>

#include <xc.h>

#include <pps.h>

void ChainInit() {
  // Initialize UART 2 at 115.2k.
  U2BRG = 151;
  U2MODE = 0x8008;
  U2STA = 0x0400;

  // Setup the TX pin.
  PPSOutput(OUT_FN_PPS_U2TX, OUT_PIN_PPS_RP20);

  // Configure DMA channel 0 to work with UART2 TX.
  DMA0PAD = (uint16_t) &U2TXREG;
  DMA0STAH = 0;
}

void ChainWrite(void const * buf, size_t len) {
  assert(len);

  // Configure the buffer, enable the DMA (one-shot) and force-start a
  // transaction.
  DMA0STAL = (uint16_t) buf;
  DMA0CNT = len - 1;
  DMA0CON = 0xE001;
  DMA0REQ = 0x801f;
}

