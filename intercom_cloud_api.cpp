#include "intercom_cloud_api.h"
#include "Particle.h"
#include "plf_utils.h"
#include "SparkFunMAX17043.h"

#define INTERCOM_CLOUD_API_TICK_INTER_MS 5000

static String my_name;
static String buddy_name;
static String buddy_id;
static String secret_key;
static String battery_lvl;

static int registryHandlerHelper(int key, String& value, bool valid, void *ctxt) {
	Intercom_CloudAPI *intercom_cloud_api = (Intercom_CloudAPI*)ctxt;

	plf_assert("icom_cloud_api NULL ptr", intercom_cloud_api);

	intercom_cloud_api->update_vars();

	return 0;
}

int Intercom_CloudAPI::set_my_name(String name) {
	_registry.set(REG_KEY_MY_NAME, name, true /*validity*/, true /*persistency*/);
  	return 0;
}

int Intercom_CloudAPI::set_buddy_name(String name) {
	String dummy_id = "-1";
	/*Erase buddy_id when setting a new name*/
	_registry.set(REG_KEY_BUDDY_ID, dummy_id, false /*validity*/, false /*persistency*/);
	_registry.set(REG_KEY_BUDDY_NAME, name, true /*validity*/, true /*persistency*/);
	return 0;
}

int Intercom_CloudAPI::erase(String name) {
	_registry.erase();
	return 0;
}

void Intercom_CloudAPI::tick(void) {
	unsigned long cur_millis = millis();
	unsigned long millis_delta;

	if (cur_millis < _prev_millis) {
		millis_delta = (~0UL) - _prev_millis + cur_millis;
	}
	else {
		millis_delta = cur_millis - _prev_millis;
	}

	if (millis_delta > INTERCOM_CLOUD_API_TICK_INTER_MS) {
		_prev_millis = cur_millis;
		update_vars();
	}
}

int Intercom_CloudAPI::update_vars(void) {
	bool valid;

	_registry.get(REG_KEY_MY_NAME, my_name, valid);
	if (!valid) {
		my_name = String();
	}

	_registry.get(REG_KEY_BUDDY_NAME, buddy_name, valid);
	if (!valid) {
		buddy_name = String();
	}

	_registry.get(REG_KEY_BUDDY_ID, buddy_id, valid);
	if (!valid) {
		buddy_id = String();
	}

	_registry.get(REG_KEY_SECRET_KEY, secret_key, valid);
	if (!valid) {
		secret_key = String();
	}

	battery_lvl = String((int)lipo.getSOC());

	return 0;
}

int Intercom_CloudAPI::enable_printgroup(String name) {
	int print_group;

	if (name.startsWith(String("default"))) {
		print_group = PRNTGRP_DFLT;
	}
	else if (name.startsWith(String("stats"))) {
		print_group = PRNTGRP_STATS;
	}
	else if (name.startsWith(String("messages"))) {
		print_group = PRNTGRP_MSGS;
	}
	else {
		PLF_PRINT(PRNTGRP_DFLT, "print_group fail\n");
		return -1;
	}

	printGroupEnable(print_group, true);

	return 0;
}

int Intercom_CloudAPI::disable_printgroup(String name) {
	int print_group;

	if (name.startsWith(String("default"))) {
		print_group = PRNTGRP_DFLT;
	}
	else if (name.startsWith(String("stats"))) {
		print_group = PRNTGRP_STATS;
	}
	else if (name.startsWith(String("messages"))) {
		print_group = PRNTGRP_MSGS;
	}
	else {
		PLF_PRINT(PRNTGRP_DFLT, "print_group fail\n");
		return -1;
	}

	printGroupEnable(print_group, false);

	return 0;
}

int Intercom_CloudAPI::set_key(String key_val) {
	_registry.set(REG_KEY_SECRET_KEY, key_val, true /*validity*/, true /*persistency*/);
  	return 0;
}

Intercom_CloudAPI::Intercom_CloudAPI(PlfRegistry& registry) : _registry(registry),_prev_millis(0) {
	int res;
	res = Particle.function("my_name", &Intercom_CloudAPI::set_my_name, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function my_name register result: %d\n", res);
	res = Particle.function("buddy_name", &Intercom_CloudAPI::set_buddy_name, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function buddy_name register result: %d\n", res);
	res = Particle.function("erase", &Intercom_CloudAPI::erase, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function erase register result: %d\n", res);
	res = Particle.function("en_prntgrp", &Intercom_CloudAPI::enable_printgroup, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function enable_printgroup register result: %d\n", res);
	res = Particle.function("dis_prntgrp", &Intercom_CloudAPI::disable_printgroup, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function disable_printgroup register result: %d\n", res);
	res = Particle.function("set_key", &Intercom_CloudAPI::set_key, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function key register result: %d\n", res);

	Particle.variable("my_name", my_name);
	Particle.variable("buddy_name", buddy_name);
	Particle.variable("buddy_id", buddy_id);
	Particle.variable("secret_key", secret_key);
	Particle.variable("battery_lvl", battery_lvl);

	_registry.registerHandler(REG_KEY_MY_NAME, registryHandlerHelper, this);
	_registry.registerHandler(REG_KEY_BUDDY_NAME, registryHandlerHelper, this);
	_registry.registerHandler(REG_KEY_BUDDY_ID, registryHandlerHelper, this);
	_registry.registerHandler(REG_KEY_SECRET_KEY, registryHandlerHelper, this);
}

	