#include <stdio.h>

#include <xc.h>

#include <FreeRTOS.h>
#include <task.h>

#include "app.h"
#include "chain.h"
#include "clock.h"
#include "demo_app.h"
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
  AppTaskInit(DemoAppInit());

  vTaskStartScheduler();
}

void vApplicationIdleHook() {
    asm volatile("PWRSAV #1");
}
