#include "intercom_volume_control.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"

#define MODULE_ID 1400

#define DEFAULT_VOL 50
#define VOL_STEP 50

#define VOL_CTRL_FSM_ALL_RELEASED 0
#define VOL_CTRL_FSM_MIN_PRESSED 1
#define VOL_CTRL_FSM_PLUS_PRESSED 2

#define LED_BAR_TIMEOUT_MS 1500

void Intercom_VolumeControl::_decVol(void) {
	if (_curVol + VOL_STEP < MAX_VOL) {
		_curVol += VOL_STEP;
	}

	PLF_PRINT(PRNTGRP_DFLT, "Vol=(255-)%d\n", (int)(MAX_VOL-_curVol));

	VS1063SetVol(_curVol);
}

void Intercom_VolumeControl::_incVol(void) {
	if (_curVol >= VOL_STEP) {
		_curVol -= VOL_STEP;
	}

	PLF_PRINT(PRNTGRP_DFLT, "Vol=(255-)%d\n", (int)(MAX_VOL-_curVol));

	VS1063SetVol(_curVol);
}

void Intercom_VolumeControl::_startTimer(void) {
	_ledTurnOffTime = millis() + LED_BAR_TIMEOUT_MS;
	_timerRunning = true;
}

void Intercom_VolumeControl::_setLedBar(void) {
	if (_fsm != VOL_CTRL_FSM_ALL_RELEASED) {
		int level=0;

		if (_curVol >= 250) {
			level = 0;
		}
		else if (_curVol >= 200) {
			level = 1;
		}
		else if (_curVol >= 150) {
			level = 2;
		}
		else if (_curVol >= 100) {
			level = 3;
		}
		else if (_curVol >= 50) {
			level = 4;
		}
		else {
			level = 5;
		}

		_intercom_buttonsAndLeds.getLedBar().setLevel(level);
	}
}

void Intercom_VolumeControl::_onTimeout(void) {
	_timerRunning = false;
	_intercom_buttonsAndLeds.getLedBar().setLevel(0);
}

Intercom_VolumeControl::Intercom_VolumeControl(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) : 
	_intercom_buttonsAndLeds(intercom_buttonsAndLeds), _curVol(DEFAULT_VOL), _fsm(VOL_CTRL_FSM_ALL_RELEASED),
	_ledTurnOffTime(0), _timerRunning(false) {

	VS1063SetVol(_curVol);
	_setLedBar();
}

void Intercom_VolumeControl::checkButtons(void) {
	bool incVolButtonPressed = _intercom_buttonsAndLeds.incVolumeButtonIsPressed();
	bool decVolButtonPressed = _intercom_buttonsAndLeds.decVolumeButtonIsPressed();

	switch (_fsm) {
		case VOL_CTRL_FSM_ALL_RELEASED:
			if (incVolButtonPressed) {
				_incVol();
				_fsm = VOL_CTRL_FSM_PLUS_PRESSED;
				_setLedBar();
			}
			else if (decVolButtonPressed) {
				_decVol();
				_fsm = VOL_CTRL_FSM_MIN_PRESSED;
				_setLedBar();
			}

			break;

		case VOL_CTRL_FSM_MIN_PRESSED:
			if (!decVolButtonPressed) {
				_fsm = VOL_CTRL_FSM_ALL_RELEASED;
				_setLedBar();
			}
		
			break;

		case VOL_CTRL_FSM_PLUS_PRESSED:
			if (!incVolButtonPressed) {
				_fsm = VOL_CTRL_FSM_ALL_RELEASED;
				_setLedBar();
			}
		
			break;

		default:
			break;
	}

	/*That last clause is to protect agains wraparound. Difference between current time and turnoff time should be reasonable*/
	if (_timerRunning) {
		unsigned long curTime = millis();
		if ((curTime > _ledTurnOffTime) && ((curTime-_ledTurnOffTime)<((~0UL)/2))) {
			_onTimeout();
		}
	}
}