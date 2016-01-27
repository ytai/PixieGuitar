#include "sync.h"

#include <assert.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include "time.h"

void * MutexCreate() {
  QueueHandle_t result = xSemaphoreCreateMutex();
  assert(result);
  return result;
}

Error MutexTake(void * mutex) {
  Duration duration = TimeUntilDeadline();
  for (;;) {
    Error e = (xSemaphoreTake(mutex, duration) == pdTRUE)
              ? ERROR_NONE
              : ERROR_TIMEOUT;
    if (e == ERROR_NONE || duration != DURATION_MAX) return e;
  }
}

void MutexGive(void * mutex) {
  if (xSemaphoreGive(mutex) != pdTRUE) assert(false);
}

void * SemaphoreCreateBinary() {
  QueueHandle_t result = xSemaphoreCreateBinary();
  assert(result);
  return result;
}

void SemaphoreGive(void * semaphore) {
  xSemaphoreGive(semaphore);
}

void SemaphoreGiveFromISR(void * semaphore) {
  BaseType_t woken = pdFALSE;
  xSemaphoreGiveFromISR(semaphore, &woken);
  if (woken != pdFALSE)	{
      vTaskSwitchContext();
  }
}

Error SemaphoreTake(void * semaphore) {
  Duration duration = TimeUntilDeadline();
  for (;;) {
    Error e = (xSemaphoreTake(semaphore, duration) == pdTRUE)
              ? ERROR_NONE
              : ERROR_TIMEOUT;
    if (e == ERROR_NONE || duration != DURATION_MAX) return e;
  }
}

bool SemaphoreTakeFromISR(void * semaphore) {
  BaseType_t woken = pdFALSE;
  BaseType_t result = xSemaphoreTakeFromISR(semaphore, &woken);
  if (woken != pdFALSE)	{
      vTaskSwitchContext();
  }
  switch (result) {
    case pdTRUE:  return true;
    case pdFALSE: return false;
    default:      assert(false);
  }
}

void * QueueCreate(size_t len, size_t element_size) {
  QueueHandle_t result = xQueueCreate(len, element_size);
  assert(result);
  return result;
}

void QueueReset(void * queue) {
  assert(queue);

  BaseType_t result = xQueueReset(queue);
  assert(result == pdPASS);
}

size_t QueueAvailableElements(void * queue) {
  assert(queue);

  return uxQueueMessagesWaiting(queue);
}

Error QueuePushBack(void * queue, void const * element) {
  assert(queue);
  assert(element);

  Duration duration = TimeUntilDeadline();
  for (;;) {
    Error e = (xQueueSendToBack(queue, element, duration) == pdTRUE)
               ? ERROR_NONE
               : ERROR_TIMEOUT;

    if (e == ERROR_NONE || duration != DURATION_MAX) return e;
  }
}

bool QueuePushBackFromISR(void * queue, void const * element) {
  assert(queue);
  assert(element);

  BaseType_t woken;
  BaseType_t result = xQueueSendToBackFromISR(queue, element, &woken);
  if (woken != pdFALSE)	{
      vTaskSwitchContext();
  }
  switch (result) {
    case pdTRUE:        return true;
    case errQUEUE_FULL: return false;
    default:            assert(false);
  }
}

Error QueuePopFront(void * queue, void * element) {
  assert(queue);
  assert(element);

  Duration duration = TimeUntilDeadline();
  for (;;) {
    Error e = (xQueueReceive(queue, element, duration) == pdTRUE)
               ? ERROR_NONE
               : ERROR_TIMEOUT;

    if (e == ERROR_NONE || duration != DURATION_MAX) return e;
  }
}

bool QueuePopFrontFromISR(void * queue, void * element) {
  assert(queue);
  assert(element);

  BaseType_t woken;
  BaseType_t result = xQueueReceiveFromISR(queue, element, &woken);
  if (woken != pdFALSE)	{
      vTaskSwitchContext();
  }
  switch (result) {
    case pdTRUE:  return true;
    case pdFALSE: return false;
    default:      assert(false);
  }
}

