#ifndef PLF_REGISTRY_H
#define PLF_REGISTRY_H

#include "Particle.h"
#include <functional>

#define REG_KEY_MY_NAME 0
#define REG_KEY_SECRET_KEY 1
#define REG_KEY_BUDDY_0_NAME 2
#define REG_KEY_BUDDY_1_NAME 3
#define REG_KEY_BUDDY_2_NAME 4
#define REG_KEY_SRVR_NAME 5

#define REG_KEY_BUDDY_0_ID 6
#define REG_KEY_BUDDY_1_ID 7
#define REG_KEY_BUDDY_2_ID 8

#define MAX_KEY_LEN 64
#define MAX_KEY_VAL 8

#define MAX_NUM_FUNS_PER_KEY 8

typedef std::function<int (int, String&, bool)> std_function_int_int_StringRef_bool_t;

typedef struct RegistryEntry_t {
	uint8_t value[MAX_KEY_LEN];
	uint32_t validKey;
} RegistryEntry_t;

typedef struct RegHandlerEntry_t {
	std_function_int_int_StringRef_bool_t *fun[MAX_NUM_FUNS_PER_KEY];
	String name;
	int topIndex;
} RegHandlerEntry_t;

class Plf_Registry {
private:
	RegistryEntry_t _registryShadow[MAX_KEY_VAL+1];
	RegHandlerEntry_t _regHandlers[MAX_KEY_VAL+1];

	bool _live;
	bool _initialized;

	void _walkHandlers(void);
	void _invokeHandler(int key, String& value, bool valid);
	int _registerHandler(int key, String name, std_function_int_int_StringRef_bool_t func);

	void _dataDump(void);
	
public:
	Plf_Registry();
	
	void init(void);

	int set(int key, String& value, bool valid);
	int get(int key, String& value, bool& valid);

	template <typename T>
    int registerHandler(int key, String name, int (T::*func)(int key, String&, bool), T *instance) {
    	using namespace std::placeholders;
      	return _registerHandler(key, name, std::bind(func, instance, _1, _2, _3));
    }

	int go(void);
	int erase(void);
};

extern Plf_Registry plf_registry;

#define PLF_REGISTRY_REGISTER_HANDLER(key, func, instance) (plf_registry.registerHandler(key, #key, func, instance))

#endif /*PLF_REGISTRY_H*/