#ifndef PLF_DATA_DUMP_H
#define PLF_DATA_DUMP_H

#include "Particle.h"
#include <functional>

#define MAX_NUM_DATA_DUMPERS 50

typedef void (user_function_void_void_t)(void);
typedef std::function<user_function_void_void_t> user_std_function_void_void_t;

typedef struct Plf_DataDumpNameFun {
	String name;
	user_std_function_void_void_t *func;
} Plf_DataDumpNameFun;

class Plf_DataDump {
public:
	template <typename T>
    void registerFunction(String name, void (T::*func)(void), T *instance) {
      _registerFunction(name, std::bind(func, instance));
    }

	int dataDump(String name);

	void listNames(void);

	Plf_DataDump();
	
private:

    void _registerFunction(String name, user_std_function_void_void_t func);

	int _lookup(String name);

	void _dataDump(void);
	
	Plf_DataDumpNameFun _dataDumpers[MAX_NUM_DATA_DUMPERS];
	int _curIdx;
};

extern Plf_DataDump dataDump;

#endif /*PLF_DATA_DUMP_H*/