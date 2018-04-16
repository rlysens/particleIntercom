#include "intercom_cloud_api.h"
#include "Particle.h"
#include "plf_utils.h"
#include "SparkFunMAX17043.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 300

#define INTERCOM_CLOUD_API_TICK_INTER_MS 5000

static String my_id;
static String buddy_0_id;
static String buddy_1_id;
static String buddy_2_id;
static String secret_key;
static String battery_pct;
static String wifi_pct;
static String srvr_name;

int Intercom_CloudAPI::_registryHandler(int key, String& value, bool valid) {
	updateVars();
	return 0;
}

void Intercom_CloudAPI::_tickerHook(void) {
	updateVars();
}

int Intercom_CloudAPI::set_my_id(String id) {
	plf_registry.set(REG_KEY_MY_ID, id, true /*validity*/);
  	return 0;
}

int Intercom_CloudAPI::set_buddy_0_id(String id) {
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_0_ID, id, true /*validity*/);
	return 0;
}

int Intercom_CloudAPI::set_buddy_1_id(String id) {
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_1_ID, id, true /*validity*/);
	return 0;
}

int Intercom_CloudAPI::set_buddy_2_id(String id) {
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_2_ID, id, true /*validity*/);
	return 0;
}

int Intercom_CloudAPI::erase(String name) {
	plf_registry.erase();
	return 0;
}

int Intercom_CloudAPI::clr_creds(String name) {
	WiFi.clearCredentials();
	return 0;
}

int Intercom_CloudAPI::set_srvr_name(String name) {
	plf_registry.set(REG_KEY_SRVR_NAME, name, true /*validity*/);
	return 0;
}

int Intercom_CloudAPI::updateVars(void) {
	bool valid;

	plf_registry.get(REG_KEY_MY_ID, my_id, valid);
	if (!valid) {
		my_id = String();
	}

	plf_registry.get(REG_KEY_BUDDY_0_ID, buddy_0_id, valid);
	if (!valid) {
		buddy_0_id = String();
	}

	plf_registry.get(REG_KEY_BUDDY_1_ID, buddy_1_id, valid);
	if (!valid) {
		buddy_1_id = String();
	}

	plf_registry.get(REG_KEY_BUDDY_2_ID, buddy_2_id, valid);
	if (!valid) {
		buddy_2_id = String();
	}

	plf_registry.get(REG_KEY_SECRET_KEY, secret_key, valid);
	if (!valid) {
		secret_key = String();
	}

	plf_registry.get(REG_KEY_SRVR_NAME, secret_key, valid);
	if (!valid) {
		srvr_name = String();
	}

	battery_pct = String(_intercom_batteryChecker.getBatteryPct());

	wifi_pct = String(_intercom_wifiChecker.getRSSIPct());

	return 0;
}

int Intercom_CloudAPI::enable_printgroup(String name) {
	int printGroup;
	bool found = false;

	for (printGroup=0; printGroup<NUM_PRINT_GROUPS; printGroup++) {
		if (name.equalsIgnoreCase(tracePrint.getPrintGroupName(printGroup))) {
			tracePrint.printGroupEnable(printGroup, true);
			found = true;
		}
	}
	
	if (!found) {
		PLF_PRINT(PRNTGRP_DFLT, "printgroup not found.");
	}

	return 0;
}

int Intercom_CloudAPI::disable_printgroup(String name) {
	int printGroup;
	bool found = false;

	for (printGroup=0; printGroup<NUM_PRINT_GROUPS; printGroup++) {
		if (name.equalsIgnoreCase(tracePrint.getPrintGroupName(printGroup))) {
			tracePrint.printGroupEnable(printGroup, false);
			found = true;
		}
	}
	
	if (!found) {
		PLF_PRINT(PRNTGRP_DFLT, "printgroup not found.");
	}

	return 0;
}

int Intercom_CloudAPI::set_key(String key_val) {
	plf_registry.set(REG_KEY_SECRET_KEY, key_val, true /*validity*/);
  	return 0;
}

int Intercom_CloudAPI::ddump(String name) {
	dataDump.dataDump(name);
  	return 0;
}

int Intercom_CloudAPI::testfun(String name) {
	PLF_PRINT(PRNTGRP_DFLT, "testfun called\n");
	WiFi.clearCredentials();
	System.reset();
  	return 0;
}

Intercom_CloudAPI::Intercom_CloudAPI(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds, 
	Intercom_WifiChecker& intercom_wifiChecker, Intercom_BatteryChecker& intercom_batteryChecker) : 
	Plf_TickerBase(INTERCOM_CLOUD_API_TICK_INTER_MS), 
	_intercom_buttonsAndLeds(intercom_buttonsAndLeds),
	_intercom_wifiChecker(intercom_wifiChecker),
	_intercom_batteryChecker(intercom_batteryChecker), _prevMillis(0) {
	
	Particle.function("my_id", &Intercom_CloudAPI::set_my_id, this);
	Particle.function("buddy_0_id", &Intercom_CloudAPI::set_buddy_0_id, this);
	Particle.function("buddy_1_id", &Intercom_CloudAPI::set_buddy_1_id, this);
	Particle.function("buddy_2_id", &Intercom_CloudAPI::set_buddy_2_id, this);
	Particle.function("erase", &Intercom_CloudAPI::erase, this);
	Particle.function("clr_creds", &Intercom_CloudAPI::clr_creds, this);
	Particle.function("en_prntgrp", &Intercom_CloudAPI::enable_printgroup, this);
	Particle.function("dis_prntgrp", &Intercom_CloudAPI::disable_printgroup, this);
	Particle.function("set_key", &Intercom_CloudAPI::set_key, this);
	Particle.function("ddump", &Intercom_CloudAPI::ddump, this);
	Particle.function("srvr_name", &Intercom_CloudAPI::set_srvr_name, this);
	Particle.function("testfun", &Intercom_CloudAPI::testfun, this);

	Particle.variable("my_id", my_id);
	Particle.variable("buddy_0_id", buddy_0_id);
	Particle.variable("buddy_1_id", buddy_1_id);
	Particle.variable("buddy_2_id", buddy_2_id);
	Particle.variable("secret_key", secret_key);
	Particle.variable("battery_pct", battery_pct);
	Particle.variable("wifi_pct", wifi_pct);
	Particle.variable("srvr_name", srvr_name);

	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_MY_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_0_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_1_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_2_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SECRET_KEY, &Intercom_CloudAPI::_registryHandler, this);
}

	