#ifndef INTERCOM_VOLUME_CONTROL_H
#define INTERCOM_VOLUME_CONTROL_H

#include "intercom_buttons_and_leds.h"
#include "Particle.h"

class Intercom_VolumeControl {
private:
	Intercom_ButtonsAndLeds& _intercom_buttonsAndLeds;
	uint32_t _curVol;
	int32_t _fsm;
	unsigned long _ledTurnOffTime;
	bool _timerRunning;

	void _incVol(void);
	void _decVol(void);
	void _setLedBar(void);

	void _startTimer(void);
	void _onTimeout(void);

public:
	Intercom_VolumeControl(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	void checkButtons(void);
};

#endif /*INTERCOM_VOLUME_CONTROL_H*/