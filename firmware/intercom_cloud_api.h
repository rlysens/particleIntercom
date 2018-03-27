#ifndef INTERCOM_CLOUD_API_H
#define INTERCOM_CLOUD_API_H

#include "plf_ticker_base.h"
#include "intercom_wifi_checker.h"
#include "intercom_battery_checker.h"
#include "intercom_buttons_and_leds.h"

class Intercom_CloudAPI : public Plf_TickerBase {
private:
	Intercom_ButtonsAndLeds& _intercom_buttonsAndLeds;
	Intercom_WifiChecker& _intercom_wifiChecker;
	Intercom_BatteryChecker& _intercom_batteryChecker;
	unsigned long _prevMillis;
	
	virtual void _tickerHook(void);
	int _registryHandler(int key, String& value, bool valid);

public:
	Intercom_CloudAPI(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds, 
		Intercom_WifiChecker& intercom_wifiChecker, Intercom_BatteryChecker& intercom_batteryChecker);

	int set_my_name(String name);
	int set_buddy_0_name(String name);
	int set_buddy_1_name(String name);
	int set_buddy_2_name(String name);
	int erase(String name);
	int clr_creds(String name);
	int enable_printgroup(String name);
	int disable_printgroup(String name);
	int set_key(String key_val);
	int list_ddump(String dummy);
	int ddump(String name);
	int set_srvr_name(String name);
	int testfun(String name);

	/*private*/
	int updateVars(void);
};

#endif /*INTERCOM_CLOUD_API_H*/