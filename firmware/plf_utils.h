#ifndef PLF_UTILS_H
#define PLF_UTILS_H

#include "Particle.h"

#define PLF_ASSERT_ENABLE

#ifdef PLF_ASSERT_ENABLE
#define plf_assert(errorString, condition) do { if (!(condition)) {Log.error(errorString); while(1);}} while (0)
#else
#define plf_assert(errorString, condition)
#endif /*PLF_ASSERT_ENABLE*/

#define PRNTGRP_DFLT 0
#define PRNTGRP_MSGS 1
#define PRNTGRP_RATETN 2
#define NUM_PRINT_GROUPS 3

class Plf_TracePrint {
private:
	// Use primary serial over USB interface for logging output. Used by PLF_PRINT
	SerialLogHandler _logHandler;
	bool _printGroupEnableFlag[NUM_PRINT_GROUPS];
	String _printGroupNames[NUM_PRINT_GROUPS];

	void _dataDump(void);

public:
	Plf_TracePrint();
	void init();
	
	String& getPrintGroupName(int printGroup);
	void printGroupEnable(int printGroup, bool enable);
	bool printGroupEnabled(int printGroup);
};

extern Plf_TracePrint tracePrint;

#define PLF_PRINT(group, ...) {if (tracePrint.printGroupEnabled((group))) {Log.info(__VA_ARGS__);}}

#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif /* ifndef MIN */

#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif /* ifndef MAX */

#define OUT
#define IN
#define INOUT

#endif /*PLF_UTILS_H*/
