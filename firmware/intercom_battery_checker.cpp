#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"
#include "plf_data_dump.h"

#define BATTERY_CHECK_FSM_BUTTON_RELEASED 0
#define BATTERY_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1500

Intercom_BatteryChecker::Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) :
	Intercom_LevelCheckerBase(intercom_buttonsAndLeds, BATTERY_CHECK_BUTTON) {
	dataDump.registerFunction("BatteryChecker", &Intercom_BatteryChecker::_dataDump, this);
}

int Intercom_BatteryChecker::_getLevel(void) {
	int batteryPct = getBatteryPct();
	
	PLF_PRINT(PRNTGRP_DFLT, "BatteryPct=%d\n", batteryPct);

	return batteryPct/17; /*Create 6 levels*/
}

int Intercom_BatteryChecker::getBatteryPct(void) {
	return lipop ? MIN((int)lipop->getSOC(), 100) : 100;
}

void Intercom_BatteryChecker::_dataDump(void) {
	int batteryPct = getBatteryPct();
	
	PLF_PRINT(PRNTGRP_DFLT, "BatteryPct=%d", batteryPct);
}