#ifndef PLF_TICKER_BASE_H
#define PLF_TICKER_BASE_H

#include "Particle.h"

class Plf_TickerBase {
private:
	unsigned long _prevMillis;
	unsigned long _periodMs;

	/*This gets invoked every periodMs milliseconds.*/
	virtual void _tickerHook(void) = 0;

public:
	Plf_TickerBase(unsigned long periodMs);	

	/*Call this from the main loop*/
	void tick(void);
};

#endif /*PLF_TICKER_BASE_H*/