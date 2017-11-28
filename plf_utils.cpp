#include "plf_utils.h"

static bool printGroupEnableFlag[NUM_PRINT_GROUPS];

bool printGroupEnabled(int printGroup) {
	plf_assert("Invalid print group", printGroup < NUM_PRINT_GROUPS);
	return printGroupEnableFlag[printGroup];
}

void printGroupEnable(int printGroup, bool enable) {
	plf_assert("Invalid print group", printGroup < NUM_PRINT_GROUPS);
	PLF_PRINT(PRNTGRP_DFLT, "print_group enable %d %d\n", printGroup, enable);
	printGroupEnableFlag[printGroup] = enable;
}
