#include "intercom_battery_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"
#include "plf_data_dump.h"

#define LOW_BATTERY_THRESHOLD_PCT 10
#define BATTERY_CHECK_TICK_PERIOD_MS 10000
#define BATTERY_CHECK_FSM_BUTTON_RELEASED 0
#define BATTERY_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1500

Intercom_BatteryChecker::Intercom_BatteryChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) :
	Intercom_LevelCheckerBase(intercom_buttonsAndLeds, BATTERY_CHECK_BUTTON), Plf_TickerBase(BATTERY_CHECK_TICK_PERIOD_MS),
	_ledBar(intercom_buttonsAndLeds.getLedBar()) {
	dataDump.registerFunction("BatteryChecker", &Intercom_BatteryChecker::_dataDump, this);
}

void Intercom_BatteryChecker::_tickerHook(void) {
	if ((getBatteryPct() < LOW_BATTERY_THRESHOLD_PCT) && (!_ledBar.isExclusive())) {
		_ledBar.breathe(1/*tOn*/, 1000/*tOff*/, 100/*rise*/, 4500/*fall*/, 2/*startIdx*/, 3/*stopIdx*/); 
	}
}

int Intercom_BatteryChecker::_getLevel(void) {
	int batteryPct = getBatteryPct();
	
	PLF_PRINT(PRNTGRP_DFLT, "BatteryPct=%d\n", batteryPct);

	return MAX(1,batteryPct/17); /*Create 6 levels. Clamp to one.*/
}

int Intercom_BatteryChecker::getBatteryPct(void) {
	return lipop ? MIN((int)lipop->getSOC(), 100) : 100;
}

void Intercom_BatteryChecker::_dataDump(void) {
	int batteryPct = getBatteryPct();
	
	PLF_PRINT(PRNTGRP_DFLT, "BatteryPct=%d", batteryPct);
}