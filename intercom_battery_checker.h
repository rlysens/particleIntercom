#ifndef INTERCOM_BATTERY_CHECKER_H
#define INTERCOM_BATTERY_CHECKER_H

#include "Particle.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_level_checker_base.h"

class Intercom_BatteryChecker : public Intercom_LevelCheckerBase {
private:
	virtual int _getLevel(void);
	
public:
	Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	int getBatteryPct(void);
};

#endif /*INTERCOM_BATTERY_CHECKER_H*/