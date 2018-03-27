#ifndef INTERCOM_ROOT_H
#define INTERCOM_ROOT_H

#include "intercom_message_handler.h"
#include "intercom_incoming.h"
#include "intercom_controller.h"
#include "intercom_buddy.h"
#include "intercom_cloud_api.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_outgoing.h"
#include "intercom_volume_control.h"
#include "intercom_battery_checker.h"
#include "intercom_wifi_checker.h"

#define LOCAL_PORT 50007

class Intercom_Root {
private:
	Intercom_MessageHandler _messageHandler;
	Intercom_Outgoing _intercom_outgoing;
	Intercom_Controller _intercom_controller;
	Intercom_Buddy _intercom_buddies[NUM_BUDDIES];
	Intercom_VolumeControl _intercom_volumeControl;
	Intercom_Incoming _intercom_incoming;
	Intercom_BatteryChecker _intercom_batteryChecker;
	Intercom_WifiChecker _intercom_wifiChecker;
	Intercom_CloudAPI _intercom_cloud_api;
	
public:
	Intercom_Root(Intercom_ButtonsAndLeds &_intercom_buttonsAndLeds);

	void loop(void);
};

#endif /*INTERCOM_ROOT_H*/