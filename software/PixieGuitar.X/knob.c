#include "knob.h"

#include <xc.h>
#include <pps.h>

#include "shell.h"

void KnobInit() {
  PPSInput(IN_FN_PPS_QEA1, IN_PIN_PPS_RP35);
  PPSInput(IN_FN_PPS_QEB1, IN_PIN_PPS_RPI34);
  QEI1IOC = 0x7000;
  QEI1CON = 0x8000;
}

uint8_t KnobGetPosition() {
  return (uint8_t) (POS1CNTL >> 2);
}

bool KnobIsPressed() {
  return !_RA2;
}

static int KnobCommand(int argc, const char **argv) {
  if (argc != 1) return -1;
  printf("Pos=%d Pressed=%d\r\n",
         (int) KnobGetPosition(),
         (int) KnobIsPressed());
  return 0;
}

SHELL_COMMAND(knob, KnobCommand, "Knob status.", "knob")
