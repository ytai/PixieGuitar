#ifndef TIME_H
#define	TIME_H

#include <stdbool.h>
#include <stdint.h>

#define DURATION_MAX UINT16_MAX

typedef uint16_t Timestamp;
typedef uint16_t Duration;

typedef struct _TimeDeadlineBlock {
  struct _TimeDeadlineBlock const * parent;
  Timestamp deadline;
} TimeDeadlineBlock;

void TimePushDeadline(Timestamp deadline, TimeDeadlineBlock * storage);
void TimePopDeadline();
Timestamp const * TimeGetDeadline();
Duration TimeUntilDeadline();
Timestamp TimeNow();
Timestamp TimeNowFromISR();
void TimeSleepUntilDeadline();

typedef struct {
  int cnt;
  TimeDeadlineBlock blk;
} __TimeDeadlineHelper;

#define DEADLINE(deadline)                                           \
for (__TimeDeadlineHelper __x                                        \
     __attribute__((cleanup(TimePopDeadline))) = { 1 };              \
     __x.cnt;                                                        \
     __x.cnt = 0)                                                    \
  for (TimePushDeadline((deadline), &__x.blk); __x.cnt; __x.cnt = 0)

#define NONBLOCKING() DEADLINE(TimeNow())

#define TIMEOUT(ticks) DEADLINE(TimeNow() + (ticks))
#define SLEEP_FOR(ticks) TIMEOUT(ticks) TimeSleepUntilDeadline()

#endif	// TIME_H

