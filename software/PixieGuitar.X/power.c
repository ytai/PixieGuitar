#include "power.h"

#include <string.h>

#include <xc.h>

#include "pin_util.h"
#include "shell.h"

#define POWER_EN_PIN   B4
#define POWER_GOOD_PIN A3

void PowerOn() {
  LAT(POWER_EN_PIN) = 1;
  TRIS(POWER_EN_PIN) = 0;
}

void PowerOff() {
  LAT(POWER_EN_PIN) = 0;
}

bool PowerIsGood() {
  return PORT(POWER_GOOD_PIN);
}

int PowerCommand(int argc, char const ** argv) {
  if (argc != 2) return -1;
  char const * subcommand = argv[1];
  if (strcmp("off", subcommand) == 0) {
    PowerOff();
    return 0;
  } else if (strcmp("good", subcommand) == 0) {
    printf("%s\r\n", PowerIsGood() ? "GOOD" : "BAD");
    return 0;
  } else {
    return -2;
  }
}

SHELL_COMMAND(power, PowerCommand, "Power control",
              "power off\r\n"
              "power good")