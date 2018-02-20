#ifndef INTERCOM_LEVEL_CHECKER_BASE_H
#define INTERCOM_LEVEL_CHECKER_BASE_H

#include "Particle.h"
#include "intercom_buttons_and_leds.h"

class Intercom_LevelCheckerBase {
private:
	Intercom_ButtonsAndLeds& _intercom_buttonsAndLeds;
	unsigned long _buttonPressStartTime;
	int32_t _fsm;
	int _buttonId;
	
	virtual int _getLevel(void)=0;
	virtual void _longPress(void);

public:
	Intercom_LevelCheckerBase(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds, int buttonId);

	void checkButton(void);
};

#endif /*INTERCOM_BATTERY_CHECKER_BASE_H*/