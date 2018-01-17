#ifndef INTERCOM_ROOT_H
#define INTERCOM_ROOT_H

#include "intercom_message_handler.h"
#include "intercom_incoming.h"
#include "intercom_controller.h"
#include "intercom_buddy.h"
#include "plf_registry.h"
#include "intercom_cloud_api.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_outgoing.h"
#include "intercom_volume_control.h"
#include "intercom_battery_checker.h"
#include "intercom_wifi_checker.h"

#define LOCAL_PORT 50007
#define REMOTE_PORT 50007
#define REMOTE_IP (IPAddress(52,26,112,44))

class Intercom_Root {
private:
	PlfRegistry _plf_registry;
	Intercom_MessageHandler _messageHandler;
	Intercom_Incoming _intercom_incoming;
	Intercom_Outgoing _intercom_outgoing;
	Intercom_Controller _intercom_controller;
	Intercom_ButtonsAndLeds _intercom_buttonsAndLeds;
	Intercom_Buddy _intercom_buddies[NUM_BUDDIES];
	Intercom_VolumeControl _intercom_volumeControl;
	Intercom_BatteryChecker _intercom_batteryChecker;
	Intercom_WifiChecker _intercom_wifiChecker;
	Intercom_CloudAPI _intercom_cloud_api;
	
public:
	Intercom_Root(void);

	void loop(void);
};

#endif /*INTERCOM_ROOT_H*/