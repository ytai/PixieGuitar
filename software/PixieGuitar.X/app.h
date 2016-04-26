#ifndef APP_H
#define	APP_H

#include <stddef.h>
#include <stdint.h>

#include "gfx.h"

#define APP_EV_MASK_AUDIO (1 << 0)
#define APP_EV_MASK_ACC   (1 << 1)
#define APP_EV_MASK_KNOB  (1 << 2)
#define APP_EV_MASK_SOC   (1 << 3)

#define APP_CMD_MIN 0x0100

typedef struct {
  uint16_t cmd;
  uint16_t arg1;
  uint16_t arg2;
} AppCommand;

typedef struct _App {
  void * instance;

  uint16_t (*OnStart)(void * instance);
  void (*OnStop)(void * instance);
  void (*OnPause)(void * instance);
  void (*OnResume)(void * instance);
  void (*OnTick)(void * instance,
                 GfxRect const * region,
                 int16_t * audio_samples,
                 int16_t acc[3],
                 int8_t knob_turn_delta,
                 int8_t knob_press_delta,
                 uint8_t soc_percent);
  void (*OnCommand)(void * instance, AppCommand const * cmd);

  // private
  struct _App * _parent;
  uint16_t _flags;
} App;

void AppTaskInit(App * main_app);

void AppSwitch(App * app);
void AppPush(App * app);
void AppPop();
void AppPostCommand(AppCommand const * cmd);

#endif  // APP_H

