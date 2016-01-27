#ifndef SYNC_H
#define	SYNC_H

#include <stdbool.h>
#include <stddef.h>

#include "base_macros.h"
#include "error.h"

void * MutexCreate();
Error MutexTake(void * mutex) WARN_UNUSED;
void MutexGive(void * mutex);

void * SemaphoreCreateBinary();
void SemaphoreGive(void * semaphore);
void SemaphoreGiveFromISR(void * semaphore);
Error SemaphoreTake(void * semaphore) WARN_UNUSED;
bool SemaphoreTakeFromISR(void * semaphore) WARN_UNUSED;

void * QueueCreate(size_t len, size_t element_size);
void QueueReset(void * queue);
size_t QueueAvailableElements(void * queue);
Error QueuePushBack(void * queue, void const * element) WARN_UNUSED;
bool QueuePushBackFromISR(void * queue, void const * element) WARN_UNUSED;
Error QueuePopFront(void * queue, void * element) WARN_UNUSED;
bool QueuePopFrontFromISR(void * queue, void * element) WARN_UNUSED;


#endif	// SYNC_H

