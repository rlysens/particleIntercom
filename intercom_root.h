#ifndef INTERCOM_ROOT_H
#define INTERCOM_ROOT_H

#include "message_handler.h"
#include "intercom_outgoing.h"
#include "intercom_incoming.h"
#include "intercom_controller.h"
#include "plf_registry.h"
#include "intercom_cloud_api.h"

#define LOCAL_PORT 50007
#define REMOTE_PORT 50007
#define REMOTE_IP (IPAddress(52,26,112,44))

class Intercom_Root {
private:
	PlfRegistry _plf_registry;
	Message_Handler _message_handler;
	Intercom_Incoming _intercom_incoming;
	Intercom_Outgoing _intercom_outgoing;
	Intercom_Controller _intercom_controller;
	Intercom_CloudAPI _intercom_cloud_api;
public:
	Intercom_Root(void);

	void loop(void);
};

#endif /*INTERCOM_ROOT_H*/