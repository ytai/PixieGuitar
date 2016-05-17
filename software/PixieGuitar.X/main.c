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
#include "main_menu_app.h"

static MainMenuApp main_menu_app;

int main(int argc, char** argv) {
  PowerOn();

  ClockInit();

  ANSELA = 0;
  ANSELB = 0;
  SPLIM = 0xFFFF;  // TODO: implement proper handling of SPLIM.

  ModuleInit();
  I2cInit();
  ShellTaskInit();
  AppTaskInit(MainMenuAppInit(&main_menu_app));

  vTaskStartScheduler();
}

void vApplicationIdleHook() {
    asm volatile("PWRSAV #1");
}
