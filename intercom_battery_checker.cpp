#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"

#define BATTERY_CHECK_FSM_BUTTON_RELEASED 0
#define BATTERY_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1500

Intercom_BatteryChecker::Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) : 
_intercom_buttonsAndLeds(intercom_buttonsAndLeds), _fsm(BATTERY_CHECK_FSM_BUTTON_RELEASED) {

}

void Intercom_BatteryChecker::checkButton(void) {
	bool buttonPressed = _intercom_buttonsAndLeds.batteryCheckButtonIsPressed();

	if (_fsm == BATTERY_CHECK_FSM_BUTTON_RELEASED) {
		if (buttonPressed) {
			int batteryPct = MIN((int)lipo.getSOC(), 100);
			int ledBarLvl = batteryPct/17; /*Create 6 levels*/
			Intercom_LedBar ledBar = _intercom_buttonsAndLeds.getLedBar();
			ledBar.setLevel(ledBarLvl);
			PLF_PRINT(PRNTGRP_DFLT, "batteryPct=%d\n", batteryPct);
			_fsm = BATTERY_CHECK_FSM_BUTTON_PRESSED;
		}
	}
	else {/*fsm=pressed*/
		if (!buttonPressed) {
			Intercom_LedBar ledBar = _intercom_buttonsAndLeds.getLedBar();
			ledBar.setLevel(0);
			_fsm = BATTERY_CHECK_FSM_BUTTON_RELEASED;
		}
	}
}