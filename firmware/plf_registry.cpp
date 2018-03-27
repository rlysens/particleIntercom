#include "plf_registry.h"
#include "plf_utils.h"
#include "plf_data_dump.h"

#define MODULE_ID 1100

#define VALID_KEY_VAL 0x10D0BABA

int Plf_Registry::set(int key, String& value, bool valid) {
	RegistryEntry_t regEntry;
	String oldValue;
	bool oldValid;

	plf_assert("Registry not initialized", _initialized);
	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	get(key, oldValue, oldValid);

	memset(regEntry.value, 0, sizeof(regEntry.value));

	/*returns a zero terminated string*/
	value.getBytes(regEntry.value, sizeof(regEntry.value));
	
	/*zero terminate*/
	regEntry.validKey = valid ? VALID_KEY_VAL : 0;

	_registryShadow[key] = regEntry;

	/*(Possibly) put into persistent memory and invoke handler for this key if there's a change*/
	if (_live &&
		((oldValid != valid) || (!oldValue.equals(value))) &&
		(_regHandlers[key].fun!=0)) {
		if (key<=MAX_PERSISTENT_KEY_VAL) {
			EEPROM.put(key*sizeof(RegistryEntry_t), regEntry);
		}

		PLF_PRINT(PRNTGRP_DFLT, "Registry change: key %d, value %s, valid %d\n", key, regEntry.value, (int)valid);
		_invokeHandler(key, value, valid);
	}

	return 0;
}

int Plf_Registry::get(int key, String& value, bool& valid) {
	RegistryEntry_t *regEntryp=0;

	plf_assert("Registry not initialized", _initialized);
	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	regEntryp = &_registryShadow[key];

	if (regEntryp->validKey == VALID_KEY_VAL) {
		value = String((const char*)(regEntryp->value));
		valid = true;
	}
	else {
		valid = false;
	}

	return 0;
}

int Plf_Registry::_registerHandler(int key, String name, std_function_int_int_StringRef_bool_t func) {
	RegHandlerEntry_t *regHandlerEntryp;

	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	auto wrapper = new std_function_int_int_StringRef_bool_t(func);

    plf_assert("_registerHandler new fail", wrapper!=NULL);

    regHandlerEntryp = &_regHandlers[key];

	plf_assert("Too many handlers", regHandlerEntryp->topIndex<MAX_NUM_FUNS_PER_KEY);
	
	regHandlerEntryp->fun[regHandlerEntryp->topIndex] = wrapper;
	regHandlerEntryp->name = name;
	++(regHandlerEntryp->topIndex);

	return 0;
}

void Plf_Registry::_invokeHandler(int key, String& value, bool valid) {
	int ii;
	RegHandlerEntry_t *regHandlerEntryp = &_regHandlers[key];

	for (ii=0; ii<regHandlerEntryp->topIndex; ++ii) {
		if (regHandlerEntryp->fun[ii]) {
			(*(regHandlerEntryp->fun[ii]))(key, value, valid);
		}
	}
}

void Plf_Registry::_walkHandlers(void) {
	int key;

	for (key=0; key<=MAX_KEY_VAL; key++) {
		if (_regHandlers[key].fun) {
			String value;
			bool valid;

			get(key, value, valid);
			PLF_PRINT(PRNTGRP_DFLT, "[%d] %s: %s, valid: %d", 
				key, _regHandlers[key].name.c_str(), valid ? value.c_str() : "X", (int)valid);
			_invokeHandler(key, value, valid);
		}
	}
}

int Plf_Registry::go(void) {
	plf_assert("Registry not initialized", _initialized);
	_live = true;
	_walkHandlers();
	return 0;
}

int Plf_Registry::erase(void) {
	plf_assert("Registry not initialized", _initialized);
	EEPROM.clear();
	memset(_registryShadow, 0, sizeof(_registryShadow));
	_walkHandlers();

	return 0;
}

Plf_Registry::Plf_Registry() : _initialized(false) {

}

void Plf_Registry::init(void) {
	int key;

	_live = false;
	memset(_regHandlers, 0, sizeof(_regHandlers));
	memset(_registryShadow, 0, sizeof(_registryShadow));

	/*Read persistent copy of registry into the shadow copy*/

	for (key=0; key<=MAX_PERSISTENT_KEY_VAL; key++) {
		EEPROM.get(key*sizeof(RegistryEntry_t), _registryShadow[key]);
	}

	dataDump.registerFunction("Registry", &Plf_Registry::_dataDump, this);

	_initialized = true;
}

void Plf_Registry::_dataDump(void) {
	int key;

	for (key=0; key<=MAX_KEY_VAL; key++) {
		if (_regHandlers[key].fun) {
			String value;
			bool valid;

			get(key, value, valid);
			PLF_PRINT(PRNTGRP_DFLT, "[%d] %s: %s, valid %d\n", 
				key, _regHandlers[key].name.c_str(), valid ? value.c_str() : "X", (int)valid);
		}
	}
}