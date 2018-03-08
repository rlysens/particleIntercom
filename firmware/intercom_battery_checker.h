#ifndef INTERCOM_BATTERY_CHECKER_H
#define INTERCOM_BATTERY_CHECKER_H

#include "Particle.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_level_checker_base.h"
#include "plf_ticker_base.h"

class Intercom_BatteryChecker : public Intercom_LevelCheckerBase, public Plf_TickerBase {
private:
	Intercom_LedBar& _ledBar;
	
	virtual int _getLevel(void);
	virtual void _tickerHook(void);

	void _dataDump(void);
	
public:
	Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	int getBatteryPct(void);
};

#endif /*INTERCOM_BATTERY_CHECKER_H*/