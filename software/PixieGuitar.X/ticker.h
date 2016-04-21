#ifndef TICKER_H
#define	TICKER_H

typedef void (*TickerCallback)();

void TickerStart(TickerCallback callback);

void TickerStop();


#endif  // TICKER_H

