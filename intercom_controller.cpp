#include "intercom_controller.h"
#include "messages.h"
#include "plf_utils.h"
#include "plf_data_dump.h"

#define MODULE_ID 400

#define INTERCOM_CONTROLLER_TICK_INTER_MS 2000

#define INTERCOM_CONTROLLER_FSM_STATE_RESTARTED 0
#define INTERCOM_CONTROLLER_FSM_STATE_STEADY 1

static int messageHandlerHelper(Intercom_Message &msg, 
  int payloadSize, void *ctxt) {
	Intercom_Controller *intercom_controllerp = (Intercom_Controller*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_controllerp);

  return intercom_controllerp->handleMessage(msg, payloadSize);
}

void Intercom_Controller::_i_am(void) {
	int numEncodedBytes;
	static i_am_t i_am;
	bool myNameIsSet;
	String myName;

	_registry.get(REG_KEY_MY_NAME, myName, myNameIsSet);
	if (!myNameIsSet)
		return;

	myName.getBytes((unsigned char*)i_am.name, sizeof(i_am.name));

	i_am.restarted = (_fsmState == INTERCOM_CONTROLLER_FSM_STATE_RESTARTED) ? 1 : 0;

	numEncodedBytes = i_am_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &i_am);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandler.send(intercom_message, I_AM_T_MSG_ID, numEncodedBytes, false);
}

int Intercom_Controller::_rx_echo_request(Intercom_Message& msg, int payloadSize) {
	static echo_request_t echo_request;
	static echo_reply_t echo_reply;
	int numEncodedBytes;
	int numDecodedBytes = echo_request_t_decode(msg.data, 0, payloadSize, &echo_request);
	uint32_t myId = _messageHandler.getMyId();

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (myId==ID_UNKNOWN)
		return -(MODULE_ID+2);

	echo_reply.source_id = myId;
	echo_reply.destination_id = echo_request.source_id;

	numEncodedBytes = echo_reply_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_reply);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandler.send(intercom_message, ECHO_REPLY_T_MSG_ID, numEncodedBytes, true);

	return 0;
}

int Intercom_Controller::_i_am_reply(Intercom_Message& msg, int payloadSize) {
	i_am_reply_t i_am_reply;
	bool myNameIsSet;
	String myName;
	int numDecodedBytes = i_am_reply_t_decode(msg.data, 0, payloadSize, &i_am_reply);

	if (numDecodedBytes < 0) {
		return -(MODULE_ID+3);
	}

	_registry.get(REG_KEY_MY_NAME, myName, myNameIsSet);
	if (!myNameIsSet) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply received while myName not set\n");
		return -(MODULE_ID+4);
	}

	if (!String((const char*)i_am_reply.name).equals(myName)) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply string mismatch\n");
		Serial.println(String((const char*)i_am_reply.name));
		Serial.println(myName);
		return -(MODULE_ID+5);
	}

	if (_messageHandler.getMyId()==ID_UNKNOWN) {
		PLF_PRINT(PRNTGRP_DFLT, "My id received %d\n", (int)i_am_reply.id);
	}

	_messageHandler.setMyId(i_am_reply.id);

	_fsmState = INTERCOM_CONTROLLER_FSM_STATE_STEADY;
	
	return 0;
}

int Intercom_Controller::handleMessage(Intercom_Message& msg, int payloadSize) {
	switch (msg.msgId) {

    case I_AM_REPLY_T_MSG_ID:
    	return _i_am_reply(msg, payloadSize);

    case ECHO_REQUEST_T_MSG_ID:
    	return _rx_echo_request(msg, payloadSize);

    default:
      return -(MODULE_ID+6);
  }

  return 0;
}

void Intercom_Controller::_tickerHook(void) {
	_i_am();
}

Intercom_Controller::Intercom_Controller(Intercom_MessageHandler& messageHandler, PlfRegistry &registry) : 
	Plf_TickerBase(INTERCOM_CONTROLLER_TICK_INTER_MS), _messageHandler(messageHandler),
	_fsmState(INTERCOM_CONTROLLER_FSM_STATE_RESTARTED), _prevMillis(0),
 	_registry(registry) {
	_messageHandler.registerHandler(I_AM_REPLY_T_MSG_ID, messageHandlerHelper, this, true);
	_messageHandler.registerHandler(ECHO_REQUEST_T_MSG_ID, messageHandlerHelper, this, true);

	dataDump.registerFunction("Controller", &Intercom_Controller::_dataDump, this);
}

void Intercom_Controller::_dataDump(void) {
	PLF_PRINT(PRNTGRP_DFLT, "FSMstate: %s", _fsmState == INTERCOM_CONTROLLER_FSM_STATE_RESTARTED ? "Restarted" : "Steady");
	PLF_PRINT(PRNTGRP_DFLT, "MyId: %d", (int)_myId);
}
