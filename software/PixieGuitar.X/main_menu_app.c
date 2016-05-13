#include "main_menu_app.h"

#include <assert.h>
#include <string.h>

#include "app_cmd.h"
#include "display.h"
#include "demo_app.h"
#include "power.h"
#include "rainbow_app.h"

static RainbowApp rainbow_app;

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

#define WHITE RGB565(0xFF, 0xFF, 0xFF)
#define BLACK RGB565(0x00, 0x00, 0x00)
#define GRAY  RGB565(0xC0, 0xC0, 0xC0)

  char const * const strings[] = {
    "First choice",
    "Second choice",
    "Launch App",
    "Rainbow Test",
    "Fifth choice",
    "Sixth choice",
    "Seventh choice",
    "Power off",
  };

  for (size_t i = 0; i < 8; ++i) {
    uint16_t h = 1535 * i / 8;
    TextWidgetInit(&instance->texts[i],
                   strings[i],
                   DISPLAY_WIDTH,
                   16,
                   BLACK,
                   Hsv2Rgb565(h, 0xFF, 0x80),
                   GRAY,
                   WHITE,
                   i == 2 ? AppCommandPush(DemoAppInit())  :
                   i == 3 ? AppCommandPush(RainbowAppInit(&rainbow_app)) :
                   i == 7 ? (AppCommand) { APP_CMD_POWER_OFF } :
                            AppCommandNop());
    instance->textp[i] = &instance->texts[i].widget;
  }

  VerticalWidgetListInit(&instance->list, instance->textp, 8);

  return WidgetAppInit(&instance->app,
                       "Main Menu",
                       MainMenuAppOnCommand,
                       &instance->list.widget);
}

