#include "main_menu_app.h"

#include <assert.h>
#include <string.h>

#include "app_cmd.h"
#include "display.h"
#include "demo_app.h"
#include "fireworks_app.h"
#include "power.h"
#include "rainbow_app.h"
#include "spinner_app.h"
#include "base_macros.h"

static DemoApp demo_app;
static RainbowApp rainbow_app;
static SpinnerApp spinner_app;
static FireworksApp fireworks_app;

static void MainMenuAppOnCommand(App * app, AppCommand const cmd) {
  switch (cmd.cmd) {
    case APP_CMD_POWER_OFF:
      PowerOff();
      break;

    default:
      assert(false);
  }
}

App * MainMenuAppInit(MainMenuApp * instance) {
  assert(instance);

  memset(instance, 0, sizeof(MainMenuApp));

  char const * const strings[] = {
    "Fireworks",
    "Rainbow",
    "Spinner",
    "Demo",
    "Tuner",
    "Visual Metronome",
    "Power off",
  };

  for (size_t i = 0; i < 7; ++i) {
    uint16_t h = 1535 * i / 7;
    instance->widgets[i] = TextWidgetInit(
        &instance->texts[i],
        strings[i],
        DISPLAY_WIDTH,
        16,
        RGB565_BLACK,
        Hsv2Rgb565(h, 0xFF, 0x80),
        RGB565_LIGHT_GRAY,
        RGB565_WHITE,
        i == 0 ? AppCommandPush(FireworksAppInit(&fireworks_app))  :
        i == 1 ? AppCommandPush(RainbowAppInit(&rainbow_app)) :
        i == 2 ? AppCommandPush(SpinnerAppInit(&spinner_app)) :
        i == 3 ? AppCommandPush(DemoAppInit(&demo_app))  :
        i == 6 ? (AppCommand) { APP_CMD_POWER_OFF } :
                 AppCommandNop());
  }

  Widget * widget = VerticalWidgetListInit(&instance->list,
                                           instance->widgets,
                                           ARRAY_LEN(instance->widgets));

  return WidgetAppInit(&instance->app,
                       "Main Menu",
                       MainMenuAppOnCommand,
                       widget);
}

