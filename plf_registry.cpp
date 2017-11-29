#include "plf_registry.h"
#include "plf_utils.h"

#define MODULE_ID 1100

#define VALID_KEY_VAL 0x10D0BABA

int PlfRegistry::set(int key, String& value, bool valid, bool persistent) {
	RegistryEntry_t regEntry;
	String oldValue;
	bool oldValid;

	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",(key<=MAX_PERSISTENT_KEY_VAL)||(!persistent));
	plf_assert("Out of range registry key",key>=0);

	get(key, oldValue, oldValid);

	/*returns a zero terminated string*/
	value.getBytes(regEntry.value, sizeof(regEntry.value));
	
	/*zero terminate*/
	regEntry.validKey = valid ? VALID_KEY_VAL : 0;

	_registryShadow[key] = regEntry;

	/*(Possibly) put into persistent memory and invoke handler for this key if there's a change*/
	if (_live &&
		((oldValid != valid) || (!oldValue.equals(value))) &&
		(_regHandlers[key].fun!=0)) {
		if (persistent) {
			EEPROM.put(key*sizeof(RegistryEntry_t), regEntry);
		}

		PLF_PRINT(PRNTGRP_DFLT, "Registry change: key %d, value %s, valid %d\n", key, regEntry.value, (int)valid);
		_invokeHandler(key, value, valid);
	}

	return 0;
}

int PlfRegistry::get(int key, String& value, bool& valid) {
	RegistryEntry_t *regEntryp=0;

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

int PlfRegistry::registerHandler(int key, RegistryHandlerFunType *fun, void* ctxt) {
	RegHandlerEntry_t *regHandlerEntryp;

	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	regHandlerEntryp = &_regHandlers[key];

	plf_assert("Too many handlers", regHandlerEntryp->topIndex<MAX_NUM_FUNS_PER_KEY);
	
	regHandlerEntryp->fun[regHandlerEntryp->topIndex] = fun;
	regHandlerEntryp->ctxt[regHandlerEntryp->topIndex] = ctxt;
	++(regHandlerEntryp->topIndex);

	return 0;
}

void PlfRegistry::_invokeHandler(int key, String& value, bool valid) {
	int ii;
	RegHandlerEntry_t *regHandlerEntryp = &_regHandlers[key];

	for (ii=0; ii<regHandlerEntryp->topIndex; ++ii) {
		if (regHandlerEntryp->fun[ii]) {
			regHandlerEntryp->fun[ii](key, value, valid, regHandlerEntryp->ctxt[ii]);
		}
	}
}

void PlfRegistry::_walkHandlers(void) {
	int key;

	for (key=0; key<=MAX_KEY_VAL; key++) {
		if (_regHandlers[key].fun) {
			String value;
			RegistryEntry_t regEntry;
			bool valid;

			get(key, value, valid);
			value.getBytes(regEntry.value, sizeof(regEntry.value));
			PLF_PRINT(PRNTGRP_DFLT, "Registry walk: key %d, value %s, valid %d\n", key, regEntry.value, (int)valid);
			_invokeHandler(key, value, valid);
		}
	}
}

int PlfRegistry::go(void) {
	_live = true;
	_walkHandlers();
	return 0;
}

int PlfRegistry::erase(void) {
	EEPROM.clear();
	memset(_registryShadow, 0, sizeof(_registryShadow));
	_walkHandlers();

	return 0;
}


PlfRegistry::PlfRegistry() : _live(false) {
	int key;

	memset(_regHandlers, 0, sizeof(_regHandlers));
	memset(_registryShadow, 0, sizeof(_registryShadow));

	/*Read persistent copy of registry into the shadow copy*/

	for (key=0; key<=MAX_PERSISTENT_KEY_VAL; key++) {
		EEPROM.get(key*sizeof(RegistryEntry_t), _registryShadow[key]);
	}
}
