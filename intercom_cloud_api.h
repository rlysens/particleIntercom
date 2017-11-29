#ifndef INTERCOM_CLOUD_API_H
#define INTERCOM_CLOUD_API_H

#include "plf_registry.h"
#include "plf_ticker_base.h"

class Intercom_CloudAPI : public Plf_TickerBase {
private:
	PlfRegistry& _registry;
	unsigned long _prevMillis;
	
	virtual void _tickerHook(void);

public:
	Intercom_CloudAPI(PlfRegistry& registry);

	int set_my_name(String name);
	int set_buddy_0_name(String name);
	int set_buddy_1_name(String name);
	int set_buddy_2_name(String name);
	int erase(String name);
	int enable_printgroup(String name);
	int disable_printgroup(String name);
	int set_key(String key_val);

	/*private*/
	int updateVars(void);
};

#endif /*INTERCOM_CLOUD_API_H*/