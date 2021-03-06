#ifndef INTERCOM_VOLUME_CONTROL_H
#define INTERCOM_VOLUME_CONTROL_H

#include "intercom_buttons_and_leds.h"
#include "Particle.h"
#include "plf_ticker_base.h"

class Intercom_VolumeControl : public Plf_TickerBase {
private:
	Intercom_ButtonsAndLeds& _intercom_buttonsAndLeds;
	Intercom_LedBar& _ledBar;
	
	uint32_t _curAtt;
	int32_t _fsm;
	unsigned long _ledTurnOffTime;
	unsigned long _volTurnOffTime;
	bool _ledTimerRunning;
	bool _volTimerRunning;
	bool _volEnabled;
	bool _ledBarExclusive;

	void _incVol(void);
	void _decVol(void);
	void _setLedBar(void);

	void _startLedTimer(void);
	void _onLedTimeout(void);
	void _startVolTimer(void);
	void _onVolTimeout(void);

	void _doEnableVol(bool enable);

	virtual void _tickerHook(void);
	
	void _dataDump(void);

public:
	Intercom_VolumeControl(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds);

	void checkButtons(void);

	void enableVol(bool enable);
};

#endif /*INTERCOM_VOLUME_CONTROL_H*/