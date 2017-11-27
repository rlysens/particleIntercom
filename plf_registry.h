#ifndef PLF_REGISTRY_H
#define PLF_REGISTRY_H

#include "Particle.h"

#define REG_KEY_MY_NAME 0
#define REG_KEY_SECRET_KEY 1
#define REG_KEY_BUDDY_0_NAME 2
#define REG_KEY_BUDDY_1_NAME 3
#define REG_KEY_BUDDY_2_NAME 4

#define MAX_PERSISTENT_KEY_VAL 4 /*Max.127 bytes persistent storage*/

#define REG_KEY_BUDDY_0_ID 5
#define REG_KEY_BUDDY_1_ID 6
#define REG_KEY_BUDDY_2_ID 7

#define MAX_KEY_VAL 7

#define MAX_NUM_FUNS_PER_KEY 8

typedef struct RegistryEntry_t {
	uint8_t value[20];
	uint32_t validKey;
} RegistryEntry_t;

typedef int (RegistryHandlerFunType)(int key, String& value, bool valid, void *ctxt);

typedef struct RegHandlerEntry_t {
	RegistryHandlerFunType *fun[MAX_NUM_FUNS_PER_KEY];
	void *ctxt[MAX_NUM_FUNS_PER_KEY];
	int top_index;
} RegHandlerEntry_t;

class PlfRegistry {
private:
	RegistryEntry_t _registryShadow[MAX_KEY_VAL+1];
	RegHandlerEntry_t _regHandlers[MAX_KEY_VAL+1];

	bool _live;

	void _walkHandlers(void);
	void _invokeHandler(int key, String& value, bool valid);

public:
	PlfRegistry();

	int set(int key, String& value, bool valid, bool persistent);
	int get(int key, String& value, bool& valid);
	int registerHandler(int key, RegistryHandlerFunType *fun, void* ctxt);
	int go(void);
	int erase(void);
};

#endif /*PLF_REGISTRY_H*/