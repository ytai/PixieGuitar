#include "shell_task.h"

#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>

#include "console.h"
#include "shell.h"
#include "read_line.h"

READ_LINE_CONFIG(80, 6)
SHELL_CONFIG(20)

void ShellTask(void * arg) {
  (void) arg;
  for (;;) {
    char * line = read_line(&read_line_config, "> ");
    if (!line) continue;
    int ret = shell_run_command(line, &shell_config);
    if (ret) {
      printf("Command failed with code: %d\r\n", ret);
    }
  }
}

void ShellTaskInit() {
  TaskHandle_t xHandle = NULL;

  read_line_init(&read_line_config);
  ConsoleInit();
  xTaskCreate(ShellTask, "shell", configMINIMAL_STACK_SIZE + 200, NULL, 1, &xHandle);
  assert(xHandle);
}

