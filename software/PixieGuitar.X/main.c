#include <stdio.h>

#include <xc.h>

#include <FreeRTOS.h>
#include <task.h>

#include "chain.h"
#include "clock.h"
#include "gui_task.h"
#include "i2c.h"
#include "module.h"
#include "power.h"
#include "shell_task.h"

int main(int argc, char** argv) {
  PowerOn();

  ClockInit();

  ANSELA = 0;
  ANSELB = 0;
  SPLIM = 0xFFFF;  // TODO: implement proper handling of SPLIM.

  ModuleInit();
  I2cInit();
  ChainInit();

  ShellTaskInit();
  GuiTaskInit();

  vTaskStartScheduler();
}

void vApplicationIdleHook() {
    asm volatile("PWRSAV #1");
}
