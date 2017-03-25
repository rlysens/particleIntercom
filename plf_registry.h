#ifndef PLF_REGISTRY_H
#define PLF_REGISTRY_H

#include "Particle.h"

#define REG_KEY_MY_NAME 0
#define REG_KEY_BUDDY_NAME 1

typedef struct RegistryEntry_t {
	uint8_t value[32];
	uint32_t validKey;
} RegistryEntry_t;

#define MAX_KEY_VAL ((127/sizeof(RegistryEntry_t))-1)

typedef int (RegistryHandlerFunType)(int key, String& value, bool valid, void *ctxt);

typedef struct RegHandlerEntry_t {
	RegistryHandlerFunType *fun;
	void *ctxt;
} RegHandlerEntry_t;

class PlfRegistry {
private:
	RegHandlerEntry_t _regHandlers[MAX_KEY_VAL+1];
	bool _live;

	void _walkHandlers(void);

public:
	PlfRegistry();

	int set(int key, String& value, bool valid);
	int get(int key, String& value, bool& valid);
	int registerHandler(int key, RegistryHandlerFunType *fun, void* ctxt);
	int go(void);
	int erase(void);
};

#endif /*PLF_REGISTRY_H*/