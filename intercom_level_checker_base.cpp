#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"

#define LEVEL_CHECK_FSM_BUTTON_RELEASED 0
#define LEVEL_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1700

Intercom_LevelCheckerBase::Intercom_LevelCheckerBase(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds, int buttonId) : 
_intercom_buttonsAndLeds(intercom_buttonsAndLeds), _fsm(LEVEL_CHECK_FSM_BUTTON_RELEASED), _buttonId(buttonId) {
}

void Intercom_LevelCheckerBase::checkButton(void) {
	bool buttonPressed = _intercom_buttonsAndLeds.buttonIsPressed(_buttonId);

	if (_fsm == LEVEL_CHECK_FSM_BUTTON_RELEASED) {
		if (buttonPressed) {
			int lvl = _getLevel();
			_intercom_buttonsAndLeds.getLedBar().setLevel(lvl);
			_fsm = LEVEL_CHECK_FSM_BUTTON_PRESSED;
		}
	}
	else {/*fsm=pressed*/
		if (!buttonPressed) {
			_intercom_buttonsAndLeds.getLedBar().setLevel(0);
			_fsm = LEVEL_CHECK_FSM_BUTTON_RELEASED;
		}
	}
}