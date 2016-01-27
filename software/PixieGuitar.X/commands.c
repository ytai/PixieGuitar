#include "shell.h"

#include <FreeRTOS.h>
#include <task.h>
#include <xc.h>

#include "util.h"

static int Reboot(int argc, const char **argv) {
  switch (argc) {
    case 1:
      asm("reset\n");
      break;

    default:
      return -1;
  }
  return 0;
}

SHELL_COMMAND(reboot, Reboot, "Reboot.", "reboot")

static int MemoryRead(int argc, const char **argv) {
  if (argc != 3) return -1;
  uint32_t addr = parse_number(argv[1]);
  uint32_t len = parse_number(argv[2]);
  uint16_t const * p = (uint16_t const *) (uint16_t) addr;
  while (len--) {
    printf("%p: 0x%04x\r\n", p, *p);
    ++p;
  }
  return 0;
}

SHELL_COMMAND(mr, MemoryRead, "Read words of memory.", "mr <addr> <word_count>")

static int MemoryWrite(int argc, const char **argv) {
  if (argc < 3) return -1;
  uint32_t addr = parse_number(argv[1]);
  uint16_t * p = (uint16_t *) (uint16_t) addr;
  argc -= 2;
  argv += 2;
  while (argc--) {
    uint16_t value = (uint16_t) parse_number(*argv++);
    *p++ = value;
  }
  return 0;
}

SHELL_COMMAND(mw, MemoryWrite, "Write words to memory.",
              "mw <addr> <word> [<word>...]")

#define MAX_TASK_COUNT 4

static int TaskCommand(int argc, const char **argv) {
  if (argc != 1) return -1;

  TaskStatus_t status[MAX_TASK_COUNT];
  uint32_t total_run_time;

  UBaseType_t count = uxTaskGetSystemState(status,
                                           MAX_TASK_COUNT,
                                           &total_run_time);
  if (count == 0) {
    printf("Too many tasks. Increase MAX_TASK_COUNT.\r\n");
    return -2;
  }

  printf("\tName\t\tState\tPri\tStack\tCPU\r\n");

  total_run_time /= 100;
  if (!total_run_time) total_run_time = 1;

  TaskStatus_t * task = &status[0];
  while (count--) {
    char state = 'X';
    switch (task->eCurrentState) {
      case eReady  :   state = 'R'; break;
      case eBlocked:   state = 'B'; break;
      case eSuspended: state = 'S'; break;
      case eDeleted:   state = 'D'; break;
    }

    uint32_t percent = task->ulRunTimeCounter / total_run_time;

    printf("%u\t%s\t\t%c\t%u\t%u\t%u%%\r\n",
           (unsigned) task->xTaskNumber,
           task->pcTaskName,
           state,
           (unsigned) task->uxCurrentPriority,
           (unsigned) task->usStackHighWaterMark,
           (unsigned) percent);
    ++task;
  }
  return 0;
}

SHELL_COMMAND(task, TaskCommand, "Print task stats.", "task")

static int HeapCommand(int argc, const char **argv) {
  if (argc != 1) return -1;

  printf("Free heap: %u\r\n", (unsigned) xPortGetFreeHeapSize());

  return 0;
}

SHELL_COMMAND(heap, HeapCommand, "Print heap status.", "heap")
