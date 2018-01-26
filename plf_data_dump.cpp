#include "plf_data_dump.h"
#include "plf_utils.h"

#define MODULE_ID 1800

Plf_DataDump dataDump;

Plf_DataDump::Plf_DataDump() : _curIdx(0) {
}

int Plf_DataDump::dataDump(String name) {
	int res=0;

	if (name.equalsIgnoreCase("All")) {
		_dataDump();
	}
	else {
		int idx = _lookup(name);

		if (idx < 0) {
			res=0;
		}
		else {
			PLF_PRINT(PRNTGRP_DFLT, "----------------");
			PLF_PRINT(PRNTGRP_DFLT, "%s:", _dataDumpers[idx].name.c_str());
			(*(_dataDumpers[idx].func))();
			PLF_PRINT(PRNTGRP_DFLT, "----------------\n");
		}
	}

	return res;
}

void Plf_DataDump::listNames(void) {
	int ii;

	PLF_PRINT(PRNTGRP_DFLT, "----------------");
	PLF_PRINT(PRNTGRP_DFLT, "DataDump modules:");
	PLF_PRINT(PRNTGRP_DFLT, "All");
	for (ii=0; ii<_curIdx; ++ii) {
		PLF_PRINT(PRNTGRP_DFLT, "%s", _dataDumpers[ii].name.c_str());
	}
	PLF_PRINT(PRNTGRP_DFLT, "----------------\n");
}

int Plf_DataDump::_lookup(String name) {
	int ii;

	for (ii=0; ii<_curIdx; ++ii) {
		if (_dataDumpers[ii].name.equalsIgnoreCase(name)) {
			return ii;
		}
	}

	return -(MODULE_ID+2);
}

void Plf_DataDump::_registerFunction(String name, user_std_function_void_void_t func) {
	plf_assert("_registerFunction NULL ptr", func!=NULL);
    plf_assert("DataDumper already registered", _lookup(name)<0);
	plf_assert("dataDump idx out of range", _curIdx < MAX_NUM_DATA_DUMPERS);

    auto wrapper = new user_std_function_void_void_t(func);

    plf_assert("_registerFunction new fail", wrapper!=NULL);

	_dataDumpers[_curIdx].name = name;
	_dataDumpers[_curIdx].func = wrapper;
	++_curIdx;
}

void Plf_DataDump::_dataDump(void) {
	int idx;

	for (idx=0; idx<_curIdx; ++idx) {
		PLF_PRINT(PRNTGRP_DFLT, "----------------");
		PLF_PRINT(PRNTGRP_DFLT, "%s:", _dataDumpers[idx].name.c_str());
		(*(_dataDumpers[idx].func))();
		PLF_PRINT(PRNTGRP_DFLT, "----------------\n");
	}
}
