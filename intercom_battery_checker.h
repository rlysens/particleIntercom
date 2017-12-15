#ifndef INTERCOM_BATTERY_CHECKER_H
#define INTERCOM_BATTERY_CHECKER_H

#include "Particle.h"
#include "intercom_buttons_and_leds.h"

class Intercom_BatteryChecker {
private:
	Intercom_ButtonsAndLeds& _intercom_buttonsAndLeds;
	int32_t _fsm;
	
public:
	Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	void checkButton(void);
};

#endif /*INTERCOM_BATTERY_CHECKER_H*/