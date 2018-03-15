#include <Particle.h>
#include "intercom_power_management.h"
#include "board.h"
#include "plf_utils.h"

Intercom_PowerManagement::Intercom_PowerManagement() {
	pinMode(POWER_DOWN_PIN, INPUT);
}	

void Intercom_PowerManagement::checkPowerSwitch(void) {
	if (digitalRead(POWER_DOWN_PIN) == LOW) {
		powerDown();
	}
}
void Intercom_PowerManagement::powerDown(void) {
	PLF_PRINT(PRNTGRP_DFLT, "Entering deep sleep...");
	System.sleep(SLEEP_MODE_DEEP, 0);
}