#include "intercom_cloud_api.h"
#include "Particle.h"
#include "plf_utils.h"
#include "SparkFunMAX17043.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 300

#define INTERCOM_CLOUD_API_TICK_INTER_MS 5000

static String my_name;
static String buddy_0_name;
static String buddy_0_id;
static String buddy_1_name;
static String buddy_1_id;
static String buddy_2_name;
static String buddy_2_id;
static String secret_key;
static String battery_pct;
static String wifi_pct;

int Intercom_CloudAPI::_registryHandler(int key, String& value, bool valid) {
	updateVars();
	return 0;
}

void Intercom_CloudAPI::_tickerHook(void) {
	updateVars();
}

int Intercom_CloudAPI::set_my_name(String name) {
	plf_registry.set(REG_KEY_MY_NAME, name, true /*validity*/, true /*persistency*/);
  	return 0;
}

int Intercom_CloudAPI::set_buddy_0_name(String name) {
	String dummyId = "-1";
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_0_ID, dummyId, false /*validity*/, false /*persistency*/);
	plf_registry.set(REG_KEY_BUDDY_0_NAME, name, true /*validity*/, true /*persistency*/);
	return 0;
}

int Intercom_CloudAPI::set_buddy_1_name(String name) {
	String dummyId = "-1";
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_1_ID, dummyId, false /*validity*/, false /*persistency*/);
	plf_registry.set(REG_KEY_BUDDY_1_NAME, name, true /*validity*/, true /*persistency*/);
	return 0;
}

int Intercom_CloudAPI::set_buddy_2_name(String name) {
	String dummyId = "-1";
	/*Erase buddy_id when setting a new name*/
	plf_registry.set(REG_KEY_BUDDY_2_ID, dummyId, false /*validity*/, false /*persistency*/);
	plf_registry.set(REG_KEY_BUDDY_2_NAME, name, true /*validity*/, true /*persistency*/);
	return 0;
}

int Intercom_CloudAPI::erase(String name) {
	plf_registry.erase();
	return 0;
}

int Intercom_CloudAPI::updateVars(void) {
	bool valid;

	plf_registry.get(REG_KEY_MY_NAME, my_name, valid);
	if (!valid) {
		my_name = String();
	}

	plf_registry.get(REG_KEY_BUDDY_0_NAME, buddy_0_name, valid);
	if (!valid) {
		buddy_0_name = String();
	}

	plf_registry.get(REG_KEY_BUDDY_0_ID, buddy_0_id, valid);
	if (!valid) {
		buddy_0_id = String();
	}

	plf_registry.get(REG_KEY_BUDDY_1_NAME, buddy_1_name, valid);
	if (!valid) {
		buddy_1_name = String();
	}

	plf_registry.get(REG_KEY_BUDDY_1_ID, buddy_1_id, valid);
	if (!valid) {
		buddy_1_id = String();
	}

	plf_registry.get(REG_KEY_BUDDY_2_NAME, buddy_2_name, valid);
	if (!valid) {
		buddy_2_name = String();
	}

	plf_registry.get(REG_KEY_BUDDY_2_ID, buddy_2_id, valid);
	if (!valid) {
		buddy_2_id = String();
	}

	plf_registry.get(REG_KEY_SECRET_KEY, secret_key, valid);
	if (!valid) {
		secret_key = String();
	}

	battery_pct = String(_intercom_batteryChecker.getBatteryPct());

	wifi_pct = String(_intercom_wifiChecker.getRSSIPct());

	return 0;
}

int Intercom_CloudAPI::enable_printgroup(String name) {
	int printgroup;

	if (name.equalsIgnoreCase(String("default"))) {
		printgroup = PRNTGRP_DFLT;
	}
	else if (name.equalsIgnoreCase(String("messages"))) {
		printgroup = PRNTGRP_MSGS;
	}
	else if (name.equalsIgnoreCase(String("ratetune"))) {
		printgroup = PRNTGRP_RATETN;
	}
	else {
		PLF_PRINT(PRNTGRP_DFLT, "printgroup fail\n");
		return -(MODULE_ID+1);
	}

	printGroupEnable(printgroup, true);

	return 0;
}

int Intercom_CloudAPI::disable_printgroup(String name) {
	int printgroup;

	if (name.equalsIgnoreCase(String("default"))) {
		printgroup = PRNTGRP_DFLT;
	}
	else if (name.equalsIgnoreCase(String("messages"))) {
		printgroup = PRNTGRP_MSGS;
	}
	else if (name.equalsIgnoreCase(String("ratetune"))) {
		printgroup = PRNTGRP_RATETN;
	}
	else {
		PLF_PRINT(PRNTGRP_DFLT, "printgroup fail\n");
		return -(MODULE_ID+2);
	}

	printGroupEnable(printgroup, false);

	return 0;
}

int Intercom_CloudAPI::set_key(String key_val) {
	plf_registry.set(REG_KEY_SECRET_KEY, key_val, true /*validity*/, true /*persistency*/);
  	return 0;
}

int Intercom_CloudAPI::list_ddump(String dummy) {
	dataDump.listNames();
  	return 0;
}

int Intercom_CloudAPI::ddump(String name) {
	dataDump.dataDump(name);
  	return 0;
}

extern retained bool enterListenMode;

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
	
	Particle.function("my_name", &Intercom_CloudAPI::set_my_name, this);
	Particle.function("buddy_0_name", &Intercom_CloudAPI::set_buddy_0_name, this);
	Particle.function("buddy_1_name", &Intercom_CloudAPI::set_buddy_1_name, this);
	Particle.function("buddy_2_name", &Intercom_CloudAPI::set_buddy_2_name, this);
	Particle.function("erase", &Intercom_CloudAPI::erase, this);
	Particle.function("en_prntgrp", &Intercom_CloudAPI::enable_printgroup, this);
	Particle.function("dis_prntgrp", &Intercom_CloudAPI::disable_printgroup, this);
	Particle.function("set_key", &Intercom_CloudAPI::set_key, this);
	Particle.function("list_ddump", &Intercom_CloudAPI::list_ddump, this);
	Particle.function("ddump", &Intercom_CloudAPI::ddump, this);
	Particle.function("testfun", &Intercom_CloudAPI::testfun, this);

	Particle.variable("my_name", my_name);
	Particle.variable("buddy_0_name", buddy_0_name);
	Particle.variable("buddy_0_id", buddy_0_id);
	Particle.variable("buddy_1_name", buddy_1_name);
	Particle.variable("buddy_1_id", buddy_1_id);
	Particle.variable("buddy_2_name", buddy_2_name);
	Particle.variable("buddy_2_id", buddy_2_id);
	Particle.variable("secret_key", secret_key);
	Particle.variable("battery_pct", battery_pct);
	Particle.variable("wifi_pct", wifi_pct);

	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_MY_NAME, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_0_NAME, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_0_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_1_NAME, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_1_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_2_NAME, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_BUDDY_2_ID, &Intercom_CloudAPI::_registryHandler, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SECRET_KEY, &Intercom_CloudAPI::_registryHandler, this);
}

	