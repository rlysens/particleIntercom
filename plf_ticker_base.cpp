#include "plf_ticker_base.h"

#define MODULE_ID 1200

Plf_TickerBase::Plf_TickerBase(unsigned long periodMs) : _prevMillis(0), _periodMs(periodMs) {
}

void Plf_TickerBase::tick(void) {
	unsigned long curMillis = millis();
	unsigned long millisDelta;

	if (curMillis < _prevMillis) {
		millisDelta = (~0UL) - _prevMillis + curMillis;
	}
	else {
		millisDelta = curMillis - _prevMillis;
	}

	if (millisDelta > _periodMs) {
		_prevMillis = curMillis;
		_tickerHook();
	}
}