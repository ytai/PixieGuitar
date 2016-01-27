#include "analog.h"

#include <assert.h>

#include <libpic30.h>
#include <PTG.h>
#include <xc.h>

#include "atomic.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"

static int16_t buffers[2][ANALOG_BUFFER_LEN + 1] __attribute__((space(ymemory)));
static unsigned current_buffer;
static AnalogCallback buffer_done_cb;
static volatile uint16_t vbat_reading;

void AnalogInit() {
  buffer_done_cb = NULL;

  // Configure DMA interrupt priority.
  _DMA1IP = configKERNEL_INTERRUPT_PRIORITY;

  // Initialize pins (AN0 and AN1,on ports A0, A1).
  ANSELA |= 0x0003;
}

void AnalogStart(AnalogCallback callback) {
  assert(callback);
  buffer_done_cb = callback;
  current_buffer = 0;

  // Enable IRQ.
  _DMA1IE = 1;

  // Initialize DMA (channel 1).
  DMA1REQ = 0x000d;                // Trigger by ADC.
  DMA1PAD = (uint16_t) &ADC1BUF0;  // Read from ADC buffer.
  DMA1STAH = 0;                    // Write to either buffer.
  DMA1STAL = (uint16_t) &buffers[0][0];
  DMA1STBH = 0;
  DMA1STBL = (uint16_t) &buffers[1][0];
  DMA1CNT = ANALOG_BUFFER_LEN;            // The actual count is +1 from that, which is what we want.
  DMA1CON = 0x8002;                // Enable DMA in continuous ping-pong mode.

  // Initialize ADC.
  AD1CON2 = 0x0000;
  AD1CON3 = 0x0009;  // Tad = Tcy/10 => 7MHz
  AD1CON4 = 0x0100;  // Enable DMA operation.
  AD1CHS0 = 0;       // Start with channel 0.
  AD1CON1 = 0x9774;  // Enable, 12-bit, linear DMA, signed frac, trigger by PTGO12, auto-sample.

  // Initialize PTG.
  PTGCON = 0x0600;  // Clocked by system clock (70MHz), divided by 7 => 10MHz.

  // Configuration of the PTG program.
  PTGC0LIM = ANALOG_BUFFER_LEN - 2;  // Number of audio samples per Vbat sample (actual count is +1).
  PTGSDLIM = 99;  // Every command will take 99+1 cycles = 10[us]. Thus, we sample ch0 every 50[us], or 20kHz.

  // The PTG program:
  _STEP0  = PTG_CMD_CTRL  & PTG_OPT_6;  // Enable step delay.
  _STEP1  = PTG_CMD_STRB  & PTG_OPT_0;  // Select ADC channel 0 (audio).
  _STEP2  = PTG_CMD_TRIG  & PTG_OPT_C;  // Trigger ADC.
  _STEP3  = PTG_CMD_CTRL  & PTG_OPT_0;  // NOP.
  _STEP4  = PTG_CMD_CTRL  & PTG_OPT_0;  // NOP.
  _STEP5  = PTG_CMD_JMPC0 & PTG_OPT_1;  // Repeat C0 times.
  _STEP6  = PTG_CMD_STRB  & PTG_OPT_0;  // Select ADC channel 0 (audio).
  _STEP7  = PTG_CMD_TRIG  & PTG_OPT_C;  // Trigger ADC.
  _STEP8  = PTG_CMD_STRB  & PTG_OPT_1;  // Select ADC channel 1 (vbat).
  _STEP9  = PTG_CMD_TRIG  & PTG_OPT_C;  // Trigger ADC.
  _STEP10 = PTG_CMD_JMP   & PTG_OPT_1;  // Repeat.

   PTGCST = 0x8080;  // Enable and start!
}

void AnalogStop() {
  // Shutdown PTG.
  PTGCST = 0;

  // Shutdown ADC.
  AD1CON1 = 0;

  // Shutdown DMA.
  DMA1CON = 0;

  // Disable / clear pending IRQ.
  _DMA1IE = 0;
  _DMA1IF = 0;

  buffer_done_cb = NULL;
}

static uint16_t SampleVbat() {
  AD1CON2 = 0x0000;
  AD1CON3 = 0x1F09;  // Tad = Tcy/10 => 7MHz, auto-convert after 31 cycles.
  AD1CON4 = 0x0000;
  AD1CHS0 = 1;       // Sample channel 1 (Vbat).
  AD1CON1 = 0x86E0;  // Enable, 12-bit, frac, trigger by software, auto-convert.
  _SAMP = 1;         // Sample.
  while (!_DONE);    // Wait for completion.
  uint16_t result = ADC1BUF0;
  AD1CON1 = 0;       // Shutdown.
  return result;
}

float AnalogGetVbat() {
  uint16_t reading = 0;
  if (buffer_done_cb) {
    // We're running.
    reading = atomic_u16_get(&vbat_reading);
  } else {
    reading = SampleVbat();
  }
  return reading * 0.000202945f;
}

void __attribute__((interrupt, no_auto_psv, naked)) _DMA1Interrupt() {
  portSAVE_CONTEXT();
  _DMA1IF = 0;
  buffer_done_cb(buffers[current_buffer]);
  atomic_u16_set(&vbat_reading,
                 (uint16_t) buffers[current_buffer][ANALOG_BUFFER_LEN] ^ 0x8000);
  current_buffer ^= 1;
  portRESTORE_CONTEXT();
}
