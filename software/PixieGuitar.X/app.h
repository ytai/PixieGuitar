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
  char const * title;

  uint16_t (*OnStart)(struct _App * instance);
  void (*OnStop)(struct _App * instance);
  void (*OnPause)(struct _App * instance);
  void (*OnResume)(struct _App * instance);
  void (*OnTick)(struct _App * instance,
                 GfxRect const * region,
                 int16_t * audio_samples,
                 int16_t acc[3],
                 int8_t knob_turn_delta,
                 int8_t knob_press_delta,
                 uint8_t soc_percent);
  void (*OnCommand)(struct _App * instance, AppCommand const * cmd);

  // private
  struct _App * _parent;
  uint16_t _flags;
} App;

void AppTaskInit(App * main_app);

AppCommand AppCommandSwitch(App * app);
AppCommand AppCommandPush(App * app);
AppCommand AppCommandPop();

void AppPostCommand(AppCommand cmd);

#endif  // APP_H

