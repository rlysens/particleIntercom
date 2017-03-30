#include "intercom_cloud_api.h"
#include "Particle.h"
#include "plf_utils.h"

static String my_name;
static String buddy_name;

static int registryHandlerHelper(int key, String& value, bool valid, void *ctxt) {
	IntercomCloudAPI *intercom_cloud_api = (IntercomCloudAPI*)ctxt;

	plf_assert("icom_cloud_api NULL ptr", intercom_cloud_api);

	intercom_cloud_api->update_vars();

	return 0;
}

int IntercomCloudAPI::set_my_name(String name) {
	_registry.set(REG_KEY_MY_NAME, name, true);
  	return 0;
}

int IntercomCloudAPI::set_buddy_name(String name) {
	_registry.set(REG_KEY_BUDDY_NAME, name, true);
	return 0;
}

int IntercomCloudAPI::erase(String name) {
	_registry.erase();
	return 0;
}

int IntercomCloudAPI::update_vars(void) {
	bool valid;
	
	_registry.get(REG_KEY_MY_NAME, my_name, valid);
	if (!valid) {
		my_name = String();
	}

	_registry.get(REG_KEY_BUDDY_NAME, buddy_name, valid);
	if (!valid) {
		buddy_name = String();
	}

	return 0;
}

int IntercomCloudAPI::enable_printgroup(String name) {
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

int IntercomCloudAPI::disable_printgroup(String name) {
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

IntercomCloudAPI::IntercomCloudAPI(PlfRegistry& registry) : _registry(registry) {
	int res;
	res = Particle.function("my_name", &IntercomCloudAPI::set_my_name, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function my_name register result: %d\n", res);
	res = Particle.function("buddy_name", &IntercomCloudAPI::set_buddy_name, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function buddy_name register result: %d\n", res);
	res = Particle.function("erase", &IntercomCloudAPI::erase, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function erase register result: %d\n", res);
	res = Particle.function("en_prntgrp", &IntercomCloudAPI::enable_printgroup, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function enable_printgroup register result: %d\n", res);
	res = Particle.function("dis_prntgrp", &IntercomCloudAPI::disable_printgroup, this);
	PLF_PRINT(PRNTGRP_DFLT, "Cloud function disable_printgroup register result: %d\n", res);

	Particle.variable("my_name", my_name);
	Particle.variable("buddy_name", buddy_name);

	_registry.registerHandler(REG_KEY_MY_NAME, registryHandlerHelper, this);
	_registry.registerHandler(REG_KEY_BUDDY_NAME, registryHandlerHelper, this);
}

	