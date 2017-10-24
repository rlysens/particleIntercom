#ifndef INTERCOM_CLOUD_API_H
#define INTERCOM_CLOUD_API_H

#include "plf_registry.h"

class Intercom_CloudAPI {
private:
	PlfRegistry& _registry;
	
public:
	Intercom_CloudAPI(PlfRegistry& registry);

	int set_my_name(String name);
	int set_buddy_name(String name);
	int erase(String name);
	int enable_printgroup(String name);
	int disable_printgroup(String name);
	int set_key(String key_val);

	/*private*/
	int update_vars(void);
};

#endif /*INTERCOM_CLOUD_API_H*/