#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"

#define BATTERY_CHECK_FSM_BUTTON_RELEASED 0
#define BATTERY_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1500

Intercom_BatteryChecker::Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) :
	Intercom_LevelCheckerBase(intercom_buttonsAndLeds, BATTERY_CHECK_BUTTON) {
}

int Intercom_BatteryChecker::_getLevel(void) {
	int batteryPct = MIN((int)lipo.getSOC(), 100);
	
	PLF_PRINT(PRNTGRP_DFLT, "BatteryPct=%d\n", batteryPct);

	return batteryPct/17; /*Create 6 levels*/
}