#include "display.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <FreeRTOS.h>
#include <portmacro.h>

#include <libpic30.h>
#include <pps.h>
#include <xc.h>

#include "error.h"
#include "pin_util.h"
#include "sync.h"

#define EN_N_PIN  B9
#define SDA_PIN   B10
#define SCK_PIN   B11
#define SS_N_PIN  B12
#define RST_N_PIN B13
#define RS_PIN    B14
#define LED_N_PIN B15

// Commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1
#define ST7735_PWCTR6  0xFC

static uint8_t madctl;
static uint8_t spi_rx_discard;
static void * event;

static void ExpectReceive(size_t count) {
  if (!count) {
    _DMA3IF = 1;
    return;
  }
  DMA3CNT = count - 1;
  DMA3CON = 0xC011;
}

static void WaitTransferComplete() {
  _DMA3IE = 1;
  while (!_DMA3IF) {
    Error e = SemaphoreTake(event);
    (void) e;
  }
  _DMA3IF = 0;
}

static void SpinTransferComplete() {
  while (!_DMA3IF);
  _DMA3IF = 0;
}

static void SpiWriteByteAsync(uint8_t c) {
  while (SPI2STATbits.SPITBF);
  SPI2BUF = c;
}

static void SpiDmaWrite(uint16_t const * buf, size_t count, bool increment) {
  SPI2STATbits.SPIEN = 0;
  SPI2CON1bits.MODE16 = 1;
  SPI2STATbits.SPIEN = 1;

  DMA2STAL = (uint16_t) buf;

  while (count) {
    uint16_t to_write = count;
    if (count > (1 << 14)) to_write = (1 << 14);

    DMA2CNT = to_write - 1;
    DMA2CON = 0xA001 | (increment ? 0x0000 : 0x0010);
    ExpectReceive(to_write);
    DMA2REQbits.FORCE = 1;
    WaitTransferComplete();
    count -= to_write;
    buf += to_write;
  }

  SPI2STATbits.SPIEN = 0;
  SPI2CON1bits.MODE16 = 0;
  SPI2STATbits.SPIEN = 1;
}

static void SpiWriteBuffer(uint16_t const * buf, size_t count) {
  SpiDmaWrite(buf, count, true);
}

static void SpiWriteRepeatedValue(uint16_t value, size_t count) {
  SpiDmaWrite(&value, count, false);
}

static void BeginCommand(uint8_t cmd) {
  LAT(RS_PIN) = 0;
  LAT(SS_N_PIN) = 0;
  ExpectReceive(1);
  SpiWriteByteAsync(cmd);
  SpinTransferComplete();
  LAT(RS_PIN) = 1;
}

static void EndCommand() {
  LAT(SS_N_PIN) = 1;
}

static void WriteCommand(uint8_t cmd, unsigned argc, ...) {
  va_list args;
  va_start(args, argc);

  BeginCommand(cmd);
  ExpectReceive(argc);
  while (argc--) {
    SpiWriteByteAsync(va_arg(args, uint8_t));
  }
  SpinTransferComplete();
  EndCommand(cmd);

  va_end(args);
}

void BeginWriteWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  WriteCommand(ST7735_CASET, 4,  // column addr set
               0x00,
               x0+0,   // XSTART
               0x00,
               x1+0);   // XEND

  WriteCommand(ST7735_RASET, 4,  // row addr set
               0x00,
               y0+0,    // YSTART
               0x00,
               y1+0);    // YEND

  BeginCommand(ST7735_RAMWR);  // write to RAM
}

static inline void InitializePeripherals() {
  // Initialize power enable pin.
  ODC(EN_N_PIN)  = 1;  // Open-drain.
  LAT(EN_N_PIN)  = 1;  // Initially, power is off.
  TRIS(EN_N_PIN) = 0;  // Set pin as output.

  // Initialize reset pin.
  LAT(RST_N_PIN)  = 0;  // Initially, hold in reset.
  TRIS(RST_N_PIN) = 0;  // Set pin as output.

  // Initialize RS pin.
  TRIS(RS_PIN)    = 0;  // Set pin as output.

  // Initialize SS pin.
  LAT(SS_N_PIN)  = 1;  // Initially, de-asserted.
  TRIS(SS_N_PIN) = 0;  // Set pin as output.

  // Initialize SPI.
  SPI2CON1 = 0x013B;  // 1:2 divider = 35MHz.
  SPI2STAT = 0x8000;
  PPSOutput(OUT_FN_PPS_SCK2, OUT_PIN_PPS_RP43);
  PPSOutput(OUT_FN_PPS_SDO2, OUT_PIN_PPS_RP42);

  // Initialize TX DMA (channel 2).
  DMA2REQ = 0x0021;
  DMA2PAD = (uint16_t) &SPI2BUF;
  DMA2STAH = 0;

  // Initialize RX DMA (channel 3).
  // This is required to prevent the SPI RX buffer from overflowing as well as
  // to detect when sending is complete.
  DMA3REQ = 0x0021;
  DMA3PAD = (uint16_t) &SPI2BUF;
  DMA3STAL = (uint16_t) &spi_rx_discard;
  DMA3STAH = 0;
  _DMA3IP = configKERNEL_INTERRUPT_PRIORITY;

  // Initialize LED PWM.
  ODC(LED_N_PIN) = 1;   // Configure pin for open-drain.
  PTPER   = 0xFFFF;     // PWM Period.
  PWMCON1 = 0x0080;     // Disable dead-time.
  FCLCON1 = 0x0003;     // Disable faults.
  FCLCON2 = 0x0003;     // Disable faults.
  FCLCON3 = 0x0003;     // Disable faults.
  IOCON1  = 0x5400;     // Enable the PWM1L pin, inverted polarity.
  IOCON2  = 0x0000;     // Disable PWM2 I/O control.
  IOCON3  = 0x0000;     // Disable PWM3 I/O control.
  PTCON   = 0x8000;     // Enable the PWM module.
}

void DisplayInit(void) {
  event = SemaphoreCreateBinary();

  InitializePeripherals();

  // Apply power.
  LAT(EN_N_PIN) = 0;
  __delay_ms(1000);

  // Release from reset.
  __delay_us(500);
  LAT(RST_N_PIN) = 1;
  __delay_us(500);

  WriteCommand(ST7735_SWRESET, 0); // software reset
  __delay_us(150);

  WriteCommand(ST7735_SLPOUT, 0);  // out of sleep mode
  __delay_us(500);

  WriteCommand(ST7735_COLMOD, 1,  // set color mode
               0x05);             // 16-bit color
  __delay_us(10);

  WriteCommand(ST7735_FRMCTR1, 3,  // frame rate control - normal mode
               0x01,  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
               0x2C,
               0x2D);

  WriteCommand(ST7735_FRMCTR2, 3,  // frame rate control - idle mode
               0x01,  // frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D)
               0x2C,
               0x2D);

  WriteCommand(ST7735_FRMCTR3, 6,  // frame rate control - partial mode
               0x01, // dot inversion mode
               0x2C,
               0x2D,
               0x01, // line inversion mode
               0x2C,
               0x2D);

  WriteCommand(ST7735_INVCTR, 1,  // display inversion control
               0x07);  // no inversion

  WriteCommand(ST7735_PWCTR1, 3,  // power control
               0xA2,
               0x02,      // -4.6V
               0x84);      // AUTO mode

  WriteCommand(ST7735_PWCTR2, 1,  // power control
               0xC5);      // VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD

  WriteCommand(ST7735_PWCTR3, 2,  // power control
               0x0A,      // Opamp current small
               0x00);      // Boost frequency

  WriteCommand(ST7735_PWCTR4, 2,  // power control
               0x8A,      // BCLK/2, Opamp current small & Medium low
               0x2A);

  WriteCommand(ST7735_PWCTR5, 2,  // power control
               0x8A,
               0xEE);

  WriteCommand(ST7735_VMCTR1, 1,  // power control
               0x0E);

  WriteCommand(ST7735_INVOFF, 0);    // don't invert display

  // normal R G B order
  // row address/col address, bottom to top refresh
  madctl = 0xC0;
  WriteCommand(ST7735_MADCTL, 1,  // memory access control (directions)
               madctl);

  WriteCommand(ST7735_COLMOD, 1,  // set color mode
               0x05);        // 16-bit color

  WriteCommand(ST7735_CASET, 4,  // column addr set
               0x00,
               0x00,   // XSTART = 0
               0x00,
               0x7F);   // XEND = 127

  WriteCommand(ST7735_RASET, 4,  // row addr set
               0x00,
               0x00,    // XSTART = 0
               0x00,
               0x9F);    // XEND = 159

  WriteCommand(ST7735_GMCTRP1, 16,
               0x0f,
               0x1a,
               0x0f,
               0x18,
               0x2f,
               0x28,
               0x20,
               0x22,
               0x1f,
               0x1b,
               0x23,
               0x37,
               0x00,
               0x07,
               0x02,
               0x10);

  WriteCommand(ST7735_GMCTRN1, 16,
               0x0f,
               0x1b,
               0x0f,
               0x17,
               0x33,
               0x2c,
               0x29,
               0x2e,
               0x30,
               0x30,
               0x39,
               0x3f,
               0x00,
               0x07,
               0x03,
               0x10);

  WriteCommand(ST7735_DISPON, 0);
  __delay_us(100);

  WriteCommand(ST7735_NORON, 0);  // normal display on
  __delay_us(10);
}

void DisplaySetBacklight(uint16_t level) {
  PDC1 = level;
}


uint8_t DisplayGetRotation() {
  return madctl;
}

void DisplaySetRotation(uint8_t m) {
  madctl = m;
  WriteCommand(ST7735_MADCTL, 1,  // memory access control (directions)
               madctl);           // row address/col address, bottom to top refresh
}

void DisplayFillRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     Rgb565 color) {
  BeginWriteWindow(x, y, x+w-1, y+h-1);
  SpiWriteRepeatedValue(color, w * h);
  EndCommand();
}

void DisplayCopyRect(uint8_t x,
                     uint8_t y,
                     uint8_t w,
                     uint8_t h,
                     Rgb565 const * data) {
  BeginWriteWindow(x, y, x+w-1, y+h-1);
  SpiWriteBuffer(data, w * h);
  EndCommand();
}

void DisplayCopyRectFrag(uint8_t x,
                         uint8_t y,
                         uint8_t w,
                         uint8_t h,
                         uint8_t stride,
                         Rgb565 const * data) {
  BeginWriteWindow(x, y, x+w-1, y+h-1);
  for (uint8_t i = 0; i < h; ++i) {
    SpiWriteBuffer(data, w);
    data += stride;
  }
  EndCommand();
}

void __attribute__((interrupt, no_auto_psv, naked)) _DMA3Interrupt() {
  portSAVE_CONTEXT();
  _DMA3IE = 0;
  SemaphoreGiveFromISR(event);
  portRESTORE_CONTEXT();
}
