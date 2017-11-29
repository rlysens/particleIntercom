#include "intercom_buddy.h"
#include "messages.h"
#include "plf_utils.h"

#define MODULE_ID 100

#define INTERCOM_BUDDY_FSM_STATE_LISTENING 1 /*Buddy is listening*/
#define INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING 0 /*Buddy is not listening*/

#define INTERCOM_BUDDY_TICK_INTER_MS 2000

static const int regKey_buddyId[NUM_BUDDIES] = {REG_KEY_BUDDY_0_ID, REG_KEY_BUDDY_1_ID, REG_KEY_BUDDY_2_ID};
static const int regKey_buddyName[NUM_BUDDIES] = {REG_KEY_BUDDY_0_NAME, REG_KEY_BUDDY_1_NAME, REG_KEY_BUDDY_2_NAME};

static int messageHandlerHelper(Intercom_Message &msg, 
  int payloadSize, void *ctxt) {
	Intercom_Buddy *intercom_buddyp = (Intercom_Buddy*)ctxt;

	plf_assert("NULL ctxt ptr", intercom_buddyp);

 	return intercom_buddyp->handleMessage(msg, payloadSize);
}

void Intercom_Buddy::_txSetBuddy(void) {
	int numEncodedBytes;
	uint32_t myId = _messageHandlerp->getMyId();
	static set_buddy_t set_buddy;

	if (myId == ID_UNKNOWN)
		return;

	/*Set my buddy server side. Keep in mind that this code runs periodically. So if one message gets lost, no problem.*/
	set_buddy.my_id = myId;
	set_buddy.buddy_id = _buddyId; /*Note that this is 0 if buddy_id is unknown*/

	numEncodedBytes = set_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &set_buddy);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, SET_BUDDY_T_MSG_ID, numEncodedBytes, true);
}

void Intercom_Buddy::_txEchoReq(void) {
	int numEncodedBytes;
	static echo_request_t echo_request;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	echo_request.source_id = myId;
	echo_request.destination_id = _buddyId;

	numEncodedBytes = echo_request_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_request);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, ECHO_REQUEST_T_MSG_ID, numEncodedBytes, true);
}

int Intercom_Buddy::_rxEchoRep(Intercom_Message& msg, int payloadSize) {
	static echo_reply_t echo_reply;
	int numDecodedBytes = echo_reply_t_decode(msg.data, 0, payloadSize, &echo_reply);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (echo_reply.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		++_echoReplyAcc;

	return 0;
}

void Intercom_Buddy::_txWhoIsReq(void) {
	int numEncodedBytes;
	static who_is_t who_is;
	String buddyName;
	bool buddyNameIsSet;

	_registryp->get(regKey_buddyName[_buddyIdx], buddyName, buddyNameIsSet);
	if (!buddyNameIsSet)
		return;

	buddyName.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	numEncodedBytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, WHO_IS_T_MSG_ID, numEncodedBytes, true);
}

int Intercom_Buddy::_rxWhoIsRep(Intercom_Message& msg, int payloadSize) {
	static who_is_reply_t who_is_reply;
	String buddyName;
	String buddyId_s;
	bool buddyNameIsSet;
	int numDecodedBytes = who_is_reply_t_decode(msg.data, 0, payloadSize, &who_is_reply);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+2);

	_registryp->get(regKey_buddyName[_buddyIdx], buddyName, buddyNameIsSet);
	if (!buddyNameIsSet) {
		return -(MODULE_ID+3);
	}

	/*zero terminate*/
	who_is_reply.name[sizeof(who_is_reply.name)-1] = 0;

	if (!String((const char*)who_is_reply.name).equals(buddyName))
		return -(MODULE_ID+4);

	if (_buddyId == ID_UNKNOWN) {
		PLF_PRINT(PRNTGRP_DFLT, "Buddy id received %d, buddyIdx %d\n", (int)who_is_reply.id, _buddyIdx);
	}

	/*Put a string version of the buddy_id in the registry*/
	buddyId_s = String(who_is_reply.id);
	_registryp->set(regKey_buddyId[_buddyIdx], buddyId_s, true /*validity*/, false /*persistency*/);
	_buddyId = who_is_reply.id;
	
	return 0;
}

void Intercom_Buddy::_fsm(void) {
	if (_fsmState == INTERCOM_BUDDY_FSM_STATE_LISTENING) {
		if (_echoReplyAcc == 0) {
			_fsmState = INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING;
			_buddyLedp->analogWrite(0); /*Off*/
			PLF_PRINT(PRNTGRP_DFLT, "buddyFSM->NotListening\n");
		}
	}
	else { /*Not Listening state:*/
		if (_echoReplyAcc > 0) {
			_fsmState = INTERCOM_BUDDY_FSM_STATE_LISTENING;
			_buddyLedp->breathe(200 /*tOn*/, 200 /*tOff*/, 1800 /*rise*/, 1800/*fall*/);
			PLF_PRINT(PRNTGRP_DFLT, "buddyFSM->Listening\n");
		}
	}

	_echoReplyAcc = 0;
}

bool Intercom_Buddy::checkButtonAndSend(void) {
	bool buttonIsPressed;

	plf_assert("IntercomBuddy not initialized", _initialized);
	
	buttonIsPressed = _intercom_buttonsAndLedsp->buddyButtonIsPressed(_buddyIdx);

	if (buttonIsPressed) {
  		if ((_messageHandlerp->getMyId() != ID_UNKNOWN) && (_buddyId != ID_UNKNOWN)) {
  			_intercom_outgoingp->transfer(_buddyId);
  		}
  	}

  	return buttonIsPressed;
}

void Intercom_Buddy::_tickerHook(void) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	_txSetBuddy();
	_txWhoIsReq();
	_txEchoReq();
	_fsm();
}

Intercom_Buddy::Intercom_Buddy() : Plf_TickerBase(INTERCOM_BUDDY_TICK_INTER_MS), _initialized(false) {
}

void Intercom_Buddy::init(Intercom_Outgoing* intercom_outgoingp, Intercom_MessageHandler* messageHandlerp, PlfRegistry* registryp, 
	Intercom_ButtonsAndLeds* intercom_buttonsAndLedsp, int buddyIdx) {

	plf_assert("NULL ptr in IntercomBuddy::init", intercom_outgoingp);
	plf_assert("NULL ptr in IntercomBuddy::init", messageHandlerp);
	plf_assert("NULL ptr in IntercomBuddy::init", registryp);
	plf_assert("NULL ptr in IntercomBuddy::init", intercom_buttonsAndLedsp);
	plf_assert("BuddyIdx out of range", buddyIdx<NUM_BUDDIES);

	_intercom_outgoingp = intercom_outgoingp;
	_messageHandlerp = messageHandlerp;
	_registryp = registryp;
	_intercom_buttonsAndLedsp = intercom_buttonsAndLedsp;
	_buddyLedp = &(intercom_buttonsAndLedsp->getBuddyLed(buddyIdx));
	_buddyIdx = buddyIdx;
	_buddyId = ID_UNKNOWN;
	_fsmState = INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING;
	_echoReplyAcc = 0;
	_prevMillis = 0;

	_messageHandlerp->registerHandler(WHO_IS_REPLY_T_MSG_ID, messageHandlerHelper, this, true);	
	_messageHandlerp->registerHandler(ECHO_REPLY_T_MSG_ID, messageHandlerHelper, this, true);

	_buddyLedp->analogWrite(0); /*Off*/

	_initialized = true;
}

int Intercom_Buddy::handleMessage(Intercom_Message& msg, int payloadSize) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	switch (msg.msg_id) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _rxWhoIsRep(msg, payloadSize);

    case ECHO_REPLY_T_MSG_ID:
    	return _rxEchoRep(msg, payloadSize);

    default:
      return -(MODULE_ID+5);
  }

  return 0;
}