#include "intercom_wifi_checker.h"
#include "SparkFunMAX17043.h"
#include "plf_utils.h"

#define WIFI_CHECK_FSM_BUTTON_RELEASED 0
#define WIFI_CHECK_FSM_BUTTON_PRESSED 1

#define MODULE_ID 1600

Intercom_WifiChecker::Intercom_WifiChecker(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) : 
	Intercom_LevelCheckerBase(intercom_buttonsAndLeds, WIFI_CHECK_BUTTON) {
}

int Intercom_WifiChecker::_getLevel(void) {
	int rssi = WiFi.RSSI();

	PLF_PRINT(PRNTGRP_DFLT, "rssi=%d\n", rssi);

	return MAX(0,(128+rssi)/22); /*Create 6 levels*/
}

int Intercom_WifiChecker::getRSSIPct(void) { /*-1..-127dB. >=0 is error*/
	return 100+MIN(0,WiFi.RSSI())*100/127;
}