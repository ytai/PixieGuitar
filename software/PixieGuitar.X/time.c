#include "time.h"

#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>

static TimeDeadlineBlock const * GetCurrentBlock() {
  return (TimeDeadlineBlock const *) xTaskGetApplicationTaskTag(NULL);
}

Timestamp const * TimeGetDeadline() {
  TimeDeadlineBlock const * const block = GetCurrentBlock();
  if (!block) return NULL;
  return &block->deadline;
}

void TimePopDeadline() {
  TimeDeadlineBlock const * const block = GetCurrentBlock();
  assert(block);
  vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t) block->parent);
}

void TimePushDeadline(Timestamp deadline, TimeDeadlineBlock * storage) {
  assert(storage);

  TimeDeadlineBlock const * const block = GetCurrentBlock();
  storage->parent = block;
  if (block) {
    // Not the first element.
    Timestamp const now = TimeNow();
    if (deadline - now < block->deadline - now) {
      storage->deadline = deadline;
    } else {
      storage->deadline = block->deadline;
    }
  } else {
    // First element.
    storage->deadline = deadline;
  }
  vTaskSetApplicationTaskTag(NULL, (TaskHookFunction_t) storage);
}

Timestamp TimeNow() {
  return xTaskGetTickCount();
}

Timestamp TimeNowFromISR() {
  return xTaskGetTickCountFromISR();
}

void TimeSleepUntilDeadline() {
  Timestamp const * deadline = TimeGetDeadline();
  assert(deadline);
  TickType_t now = TimeNow();
  vTaskDelayUntil(&now, *deadline - now);
}

Duration TimeUntilDeadline() {
  Timestamp const * deadline = TimeGetDeadline();
  if (!deadline) return DURATION_MAX;
  return *deadline - TimeNow();
}
