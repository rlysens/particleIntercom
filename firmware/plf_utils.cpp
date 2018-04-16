#include "plf_utils.h"
#include "plf_data_dump.h"

#define MODULE_ID 1300

bool Plf_TracePrint::printGroupEnabled(int printGroup) {
	plf_assert("Invalid print group", printGroup < NUM_PRINT_GROUPS);
	return _printGroupEnableFlag[printGroup];
}

void Plf_TracePrint::printGroupEnable(int printGroup, bool enable) {
	plf_assert("Invalid print group", printGroup < NUM_PRINT_GROUPS);
	PLF_PRINT(PRNTGRP_DFLT, "print_group enable %d %d\n", printGroup, enable);
	_printGroupEnableFlag[printGroup] = enable;
}

Plf_TracePrint::Plf_TracePrint() {
	_printGroupNames[PRNTGRP_DFLT] = String("default");
	_printGroupNames[PRNTGRP_MSGS] = String("messages");
	_printGroupNames[PRNTGRP_RATETN] = String("ratetune");
	
	_printGroupEnableFlag[PRNTGRP_DFLT] = true;
}

void Plf_TracePrint::init(void) {
	dataDump.registerFunction("TracePrint", &Plf_TracePrint::_dataDump, this);
}

void Plf_TracePrint::_dataDump(void) {
	int ii;

	for (ii=0; ii<NUM_PRINT_GROUPS; ii++) {
		PLF_PRINT(PRNTGRP_DFLT, "Printgroup %s: %s", _printGroupNames[ii].c_str(), _printGroupEnableFlag[ii] ? "Enabled" : "Disabled");
	}
}

String& Plf_TracePrint::getPrintGroupName(int printGroup) {
	plf_assert("Invalid print group", printGroup < NUM_PRINT_GROUPS);
	return _printGroupNames[printGroup];
}
