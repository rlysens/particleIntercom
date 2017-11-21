#include "plf_registry.h"
#include "plf_utils.h"

#define VALID_KEY_VAL 0x10D0BABA

int PlfRegistry::set(int key, String& value, bool valid, bool persistent) {
	RegistryEntry_t reg_entry;
	String old_value;
	bool old_valid;

	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	get(key, old_value, old_valid);

	/*returns a zero terminated string*/
	value.getBytes(reg_entry.value, sizeof(reg_entry.value));
	
	/*zero terminate*/
	reg_entry.validKey = valid ? VALID_KEY_VAL : 0;

	_registryShadow[key] = reg_entry;

	/*(Possibly) put into persistent memory and invoke handler for this key if there's a change*/
	if ( _live &&
		((old_valid != valid) || (!old_value.equals(value))) &&
		(_regHandlers[key].fun!=0)) {
		if (persistent) {
			EEPROM.put(key*sizeof(RegistryEntry_t), reg_entry);
		}

		PLF_PRINT(PRNTGRP_DFLT, "Registry change: key %d, value %s, valid %d\n", key, reg_entry.value, (int)valid);
		_invokeHandler(key, value, valid);
	}

	return 0;
}

int PlfRegistry::get(int key, String& value, bool& valid) {
	RegistryEntry_t *reg_entryp=0;

	plf_assert("Out of range registry key",key<=MAX_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	reg_entryp = &_registryShadow[key];

	if (reg_entryp->validKey == VALID_KEY_VAL) {
		value = String((const char*)(reg_entryp->value));
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

	regHandlerEntryp->fun[regHandlerEntryp->top_index] = fun;
	regHandlerEntryp->ctxt[regHandlerEntryp->top_index] = ctxt;
	++(regHandlerEntryp->top_index);

	return 0;
}

void PlfRegistry::_invokeHandler(int key, String& value, bool valid) {
	int ii;
	RegHandlerEntry_t *regHandlerEntryp = &_regHandlers[key];

	for (ii=0; ii<regHandlerEntryp->top_index; ++ii) {
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
			RegistryEntry_t reg_entry;
			bool valid;

			get(key, value, valid);
			value.getBytes(reg_entry.value, sizeof(reg_entry.value));
			PLF_PRINT(PRNTGRP_DFLT, "Registry walk: key %d, value %s, valid %d\n", key, reg_entry.value, (int)valid);
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

	/*Read persistent copy of registry into the shadow copy*/

	for (key=0; key<=MAX_KEY_VAL; key++) {
		EEPROM.get(key*sizeof(RegistryEntry_t), _registryShadow[key]);
	}
}
