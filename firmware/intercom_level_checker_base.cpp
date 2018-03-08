#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"

#define LEVEL_CHECK_FSM_BUTTON_RELEASED 0
#define LEVEL_CHECK_FSM_BUTTON_PRESSED 1

#define LONG_PRESS_TIME_MS 3000

#define MODULE_ID 1700

Intercom_LevelCheckerBase::Intercom_LevelCheckerBase(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds, int buttonId) : 
_intercom_buttonsAndLeds(intercom_buttonsAndLeds), _buttonPressStartTime(0), _fsm(LEVEL_CHECK_FSM_BUTTON_RELEASED), _buttonId(buttonId),
_gotLedBarExclusive(false), _ledBar(intercom_buttonsAndLeds.getLedBar()) {
}

void Intercom_LevelCheckerBase::_longPress(void) {
}

void Intercom_LevelCheckerBase::checkButton(void) {
	bool buttonPressed = _intercom_buttonsAndLeds.buttonIsPressed(_buttonId);
	
	if (_fsm == LEVEL_CHECK_FSM_BUTTON_RELEASED) {
		if (buttonPressed) {
			_buttonPressStartTime = millis(); 
			_gotLedBarExclusive |= _ledBar.setExclusive(true);
			if (_gotLedBarExclusive) {
				_ledBar.reset();
				int lvl = _getLevel();
				_ledBar.setLevel(lvl);
			}
			_fsm = LEVEL_CHECK_FSM_BUTTON_PRESSED;
		}
	}
	else {/*fsm=pressed*/
		if (buttonPressed) {
			if (millis() - _buttonPressStartTime > LONG_PRESS_TIME_MS) {
				_longPress();
			}
		}
		else {
			if (_gotLedBarExclusive) {
				_ledBar.setLevel(0);
				_ledBar.setExclusive(false);
				_gotLedBarExclusive = false;
			}

			_fsm = LEVEL_CHECK_FSM_BUTTON_RELEASED;
		}
	}
}