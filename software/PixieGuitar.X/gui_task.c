#include "gui_task.h"

#include <assert.h>
#include <stdio.h>

#include <dsp.h>

#include <FreeRTOS.h>
#include <task.h>

#include "analog.h"
#include "display.h"
#include "gfx.h"
#include "knob.h"
#include "sync.h"
#include "time.h"
#include "chain.h"
#include "imu.h"

static void * buffer_queue;

static fractcomplex twiddle[ANALOG_BUFFER_LEN / 2] __attribute__((space(xmemory)));
static fractcomplex complex_buf[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));
static fractional window[ANALOG_BUFFER_LEN] __attribute__((space(ymemory)));

static void AnalogDone(int16_t * audio) {
  if (!QueuePushBackFromISR(buffer_queue, &audio)) {
    printf("Overflow\r\n");
  }
}

static uint8_t pixie_buf[3];

static uint8_t Tri(uint8_t x) {
  if (x < 43) return x * 6;
  if (x < 128) return 255;
  if (x < 171) return 1023 - x * 6;
  return 0;
}

static void PixieSetColor(uint8_t hue, uint8_t brightness) {
  pixie_buf[0] = ((uint16_t) Tri(hue +   0) * brightness) >> 8;
  pixie_buf[1] = ((uint16_t) Tri(hue +  85) * brightness) >> 8;
  pixie_buf[2] = ((uint16_t) Tri(hue + 170) * brightness) >> 8;
}

void GuiTask(void * arg) {
  (void) arg;

  TwidFactorInit(ANALOG_LOG2_BUFFER_LEN, twiddle, 0);
  HanningInit(ANALOG_BUFFER_LEN, window);

  ImuInit();
  ImuOn();

  static uint16_t area[16][16];
  for (unsigned i = 0; i < 16; ++i)
    for (unsigned j = 0; j < 16; ++j)
      area[i][j] = RGB565(0xff, 0xff, 0xff);

  DisplayInit();
  DisplaySetBacklight(0xFFFF);

  GfxFillScreen(RGB565(0, 0, 0));

  AnalogStart(AnalogDone);

  for (;;) {
    TIMEOUT(1500) {
      int16_t * buf;
      Error e = QueuePopFront(buffer_queue, &buf);
      uint8_t knob = (uint8_t) KnobGetPosition();
      bool pressed = KnobIsPressed();
      float vbat = AnalogGetVbat();
      char vbat_str[13];
      snprintf(vbat_str, sizeof(vbat_str), "V=%1.2f P=%03d", vbat, knob);
      if (ERROR_NONE == e) {
        VectorWindow(ANALOG_BUFFER_LEN, buf, buf, window);
        for (unsigned i = 0; i < ANALOG_BUFFER_LEN; ++i) {
          complex_buf[i].real = buf[i] * knob;
          complex_buf[i].imag = 0;
        }

        FFTComplexIP(ANALOG_LOG2_BUFFER_LEN, complex_buf, twiddle, COEFFS_IN_DATA);
        BitReverseComplex(ANALOG_LOG2_BUFFER_LEN, complex_buf);
        SquareMagnitudeCplx(ANALOG_BUFFER_LEN, complex_buf, buf);

        uint16_t max_volume = 0;
        uint8_t max_index = 0;
        for (uint8_t i = 0; i < 64; ++i) {
          if (buf[i + 4] > max_volume) {
            max_index = i;
            max_volume = buf[i + 4];
          }
        }
        max_volume *= knob;
        if (max_volume > 255) max_volume = 255;
        PixieSetColor(max_index << 2, max_volume);
        ChainWrite(pixie_buf, sizeof(pixie_buf));

        for (size_t y = 16; y < 160; ++y) {
          int16_t sample = buf[y - 16];
//          uint8_t x = sample >> 9 ;
//          line[x] = RGB565(0xff, 0xff, 0x00);
//          DisplayCopyRect(0, y, 128, 1, line);
//          line[x] = 0;
          uint8_t x = sample * knob;
          if (x > 128) x = 128;
          DisplayFillRect(0, y, x, 1, RGB565(0xff, 0xff, 0x00));
          DisplayFillRect(x, y, 128 - x, 1, RGB565(0x00, 0x00, 0x00));
        }
      } else {
        printf("Underflow\r\n");
      }
      GfxDrawString(10,
                    1,
                    vbat_str,
                    pressed ? RGB565(0x00, 0xff, 0x00) : RGB565(0xff, 0xff, 0xff),
                    RGB565(0, 0, 0));

      int16_t imu[3];
      ImuRead(imu);
      unsigned x = ((imu[0] ^ 0x4000) >> 11) & 0xF;
      unsigned y = ((imu[1] ^ 0x4000) >> 11) & 0xF;
      area[y][x] = RGB565(0xff, 0x00, 0x00);
      DisplayCopyRect(112, 0, 16, 16, area);
      area[y][x] = RGB565(0xff, 0xff, 0xff);
    }
  }
}

void GuiTaskInit() {
  AnalogInit();
  KnobInit();

  buffer_queue = QueueCreate(1, sizeof(int16_t *));
  assert(buffer_queue);

  TaskHandle_t xHandle = NULL;
  xTaskCreate(GuiTask, "gui", configMINIMAL_STACK_SIZE + 100, NULL, 2, &xHandle);
  assert(xHandle);
}

