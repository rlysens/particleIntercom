#ifndef INTERCOM_WIFI_CHECKER_H
#define INTERCOM_WIFI_CHECKER_H

#include "Particle.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_level_checker_base.h"

class Intercom_WifiChecker : public Intercom_LevelCheckerBase {
private:
	virtual int _getLevel(void);
	
public:
	Intercom_WifiChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	int getRSSI(void); /*-1..-127dB. >=0 is error*/
};

#endif /*INTERCOM_WIFI_CHECKER_H*/