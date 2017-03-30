#include "plf_utils.h"

static bool printGroupEnableFlag[NUM_PRINT_GROUPS];

bool printGroupEnabled(int print_group) {
	plf_assert("Invalid print group", print_group < NUM_PRINT_GROUPS);
	return printGroupEnableFlag[print_group];
}

void printGroupEnable(int print_group, bool enable) {
	plf_assert("Invalid print group", print_group < NUM_PRINT_GROUPS);
	PLF_PRINT(PRNTGRP_DFLT, "print_group enable %d %d\n", print_group, enable);
	printGroupEnableFlag[print_group] = enable;
}
