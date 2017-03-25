#include "plf_registry.h"
#include "plf_utils.h"

#define VALID_KEY_VAL 0x10D0BABA

int PlfRegistry::set(int key, String& value, bool valid) {
	RegistryEntry_t reg_entry;
	String old_value;
	bool old_valid;

	plf_assert("Out of range registry key",key<=VALID_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	get(key, old_value, old_valid);

	value.getBytes(reg_entry.value, sizeof(reg_entry.value));
	
	/*zero terminate*/
	reg_entry.value[sizeof(reg_entry.value)-1] = 0;
	reg_entry.validKey = valid ? VALID_KEY_VAL : 0;

	EEPROM.put(key*sizeof(RegistryEntry_t), reg_entry);

	/*Invoked handler for this key if there's a change*/
	if ( _live &&
		((old_valid != valid) || (!old_value.equals(value))) &&
		(_regHandlers[key].fun!=0)) {
		_regHandlers[key].fun(key, value, valid, _regHandlers[key].ctxt);
	}

	return 0;
}

int PlfRegistry::get(int key, String& value, bool& valid) {
	RegistryEntry_t reg_entry;

	plf_assert("Out of range registry key",key<=VALID_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	EEPROM.get(key*sizeof(RegistryEntry_t), reg_entry);

	if (reg_entry.validKey == VALID_KEY_VAL) {
		value = String((const char*)reg_entry.value);
		valid = true;
	}
	else {
		valid = false;
	}

	return 0;
}

int PlfRegistry::registerHandler(int key, RegistryHandlerFunType *fun, void* ctxt) {

	plf_assert("Out of range registry key",key<=VALID_KEY_VAL);
	plf_assert("Out of range registry key",key>=0);

	_regHandlers[key].fun = fun;
	_regHandlers[key].ctxt = ctxt;

	return 0;
}

void PlfRegistry::_walkHandlers(void) {
	int key;

	for (key=0; key<=MAX_KEY_VAL; key++) {
		if (_regHandlers[key].fun) {
			String value;
			bool valid;

			get(key, value, valid);
			_regHandlers[key].fun(key, value, valid, _regHandlers[key].ctxt);
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
	_walkHandlers();

	return 0;
}


PlfRegistry::PlfRegistry() : _live(false) {
	memset(_regHandlers, 0, sizeof(_regHandlers));
}
