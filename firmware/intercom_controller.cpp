#include "intercom_controller.h"
#include "messages.h"
#include "plf_utils.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 400

#define INTERCOM_CONTROLLER_TICK_INTER_MS 2000

void Intercom_Controller::_i_am(void) {
	int numEncodedBytes;
	static i_am_t i_am;
	uint32_t myId = _messageHandler.getMyId(); /*Returns ID_UNKNOWN if unknown*/
	
	if (myId == ID_UNKNOWN) {
		return;
	}

	i_am.my_id = myId;
	i_am.srvr_addr = _myServerAddress.raw().ipv4;

	numEncodedBytes = i_am_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &i_am);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandler.send(intercom_message, I_AM_T_MSG_ID, INTERCOM_SERVER_ID, numEncodedBytes, _myServerAddress);
}

int Intercom_Controller::handleMessage(Intercom_Message& msg, int payloadSize) {
	switch (msg.msgId) {
	    case KEEP_ALIVE_T_MSG_ID:
	    	return 0;

	    default:
	      return -(MODULE_ID+6);
  	}

  	return 0;
}

void Intercom_Controller::_tickerHook(void) {
	_i_am();
}

int Intercom_Controller::_registryHandlerSrvrAddr(int key, String& value, bool valid) {
  plf_assert("invalid reg key", key == REG_KEY_SRVR_NAME);

  if (valid) {
  	int resolveAttempts = 0;

    do {
    	_myServerAddress = WiFi.resolve(value);
    	if (!_myServerAddress) {
      		PLF_PRINT(PRNTGRP_DFLT, "Could not resolve server name.");
    	}
	} while ((!_myServerAddress) && (resolveAttempts++ < 5));

    String serverAddrString = String(_myServerAddress.raw().ipv4);
    plf_registry.set(REG_KEY_SRVR_ADDR, serverAddrString, _myServerAddress);
  }

  return 0;
}

Intercom_Controller::Intercom_Controller(Intercom_MessageHandler& messageHandler) : 
	Plf_TickerBase(INTERCOM_CONTROLLER_TICK_INTER_MS), _messageHandler(messageHandler) {
	_messageHandler.registerHandler(KEEP_ALIVE_T_MSG_ID, &Intercom_Controller::handleMessage, this, true);

	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SRVR_NAME, &Intercom_Controller::_registryHandlerSrvrAddr, this);
	dataDump.registerFunction("Controller", &Intercom_Controller::_dataDump, this);
}

void Intercom_Controller::_dataDump(void) {
	PLF_PRINT(PRNTGRP_DFLT, "MyServerAddress: %s", _myServerAddress.toString().c_str());
}
