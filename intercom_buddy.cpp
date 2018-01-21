#include "intercom_buddy.h"
#include "messages.h"
#include "plf_utils.h"
#include "vs1063a_codec.h"

#define MODULE_ID 100

#define INTERCOM_BUDDY_LISTENING_STATE_LISTENING 1 /*Buddy is listening*/
#define INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING 0 /*Buddy is not listening*/

#define INTERCOM_BUDDY_BUTTON_STATE_RELEASED 0
#define INTERCOM_BUDDY_BUTTON_STATE_PRESSED 1

#define INTERCOM_BUDDY_COMM_STATE_STARTED 0
#define INTERCOM_BUDDY_COMM_STATE_STOPPED 1

#define INTERCOM_BUDDY_LED_STATE_OFF 0
#define INTERCOM_BUDDY_LED_STATE_BREATHING 1
#define INTERCOM_BUDDY_LED_STATE_BLINKING 2

#define INTERCOM_BUDDY_TICK_INTER_MS 500

#define RECORD_REQ_ID_BUTTON 0
#define RECORD_REQ_ID_INCOMING_COMM 1

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

int Intercom_Buddy::_rxCommStart(Intercom_Message& msg, int payloadSize) {
	static comm_start_t comm_start;
	static comm_start_ack_t comm_start_ack;
	int numEncodedBytes;
	int numDecodedBytes = comm_start_t_decode(msg.data, 0, payloadSize, &comm_start);
	uint32_t myId = _messageHandlerp->getMyId();

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (myId==ID_UNKNOWN)
		return -(MODULE_ID+2);

	if (comm_start.source_id != (int32_t)_buddyId) /*Did this buddy send it?*/
		return 0;

	_intercom_outgoingp->recordRequest(RECORD_REQ_ID_INCOMING_COMM, true);

	_commState = INTERCOM_BUDDY_COMM_STATE_STARTED;

	comm_start_ack.source_id = myId;
	comm_start_ack.destination_id = comm_start.source_id;

	numEncodedBytes = comm_start_ack_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_start_ack);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_START_ACK_T_MSG_ID, numEncodedBytes, true);

	return 0;
}

int Intercom_Buddy::_rxCommStop(Intercom_Message& msg, int payloadSize) {
	static comm_stop_t comm_stop;
	static comm_stop_ack_t comm_stop_ack;
	int numEncodedBytes;
	int numDecodedBytes = comm_stop_t_decode(msg.data, 0, payloadSize, &comm_stop);
	uint32_t myId = _messageHandlerp->getMyId();

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (myId==ID_UNKNOWN)
		return -(MODULE_ID+2);

	if (comm_stop.source_id != (int32_t)_buddyId) /*Did this buddy send it?*/
		return 0;

	_intercom_outgoingp->recordRequest(RECORD_REQ_ID_INCOMING_COMM, false);

	_commState = INTERCOM_BUDDY_COMM_STATE_STOPPED;

	comm_stop_ack.source_id = myId;
	comm_stop_ack.destination_id = comm_stop.source_id;

	numEncodedBytes = comm_stop_ack_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_stop_ack);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_STOP_ACK_T_MSG_ID, numEncodedBytes, true);

	return 0;
}

void Intercom_Buddy::_txCommStart(void) {
	int numEncodedBytes;
	static comm_start_t comm_start;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	comm_start.source_id = myId;
	comm_start.destination_id = _buddyId;

	numEncodedBytes = comm_start_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_start);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_START_T_MSG_ID, numEncodedBytes, true);
}

int Intercom_Buddy::_rxCommStartAck(Intercom_Message& msg, int payloadSize) {
	static comm_start_ack_t comm_start_ack;
	int numDecodedBytes = comm_start_ack_t_decode(msg.data, 0, payloadSize, &comm_start_ack);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (comm_start_ack.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		_sendCommStart = false; /*Stop sending comm_start*/

	return 0;
}

void Intercom_Buddy::_txCommStop(void) {
	int numEncodedBytes;
	static comm_stop_t comm_stop;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	comm_stop.source_id = myId;
	comm_stop.destination_id = _buddyId;

	numEncodedBytes = comm_stop_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_stop);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_STOP_T_MSG_ID, numEncodedBytes, true);
}

int Intercom_Buddy::_rxCommStopAck(Intercom_Message& msg, int payloadSize) {
	static comm_stop_ack_t comm_stop_ack;
	int numDecodedBytes = comm_stop_ack_t_decode(msg.data, 0, payloadSize, &comm_stop_ack);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (comm_stop_ack.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		_sendCommStop = false; /*Stop sending comm_stop*/

	return 0;
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

void Intercom_Buddy::_fsmUpdate(void) {
#if 0
	PLF_PRINT(PRNTGRP_DFLT, "echoReplyAcc=%d\n", _echoReplyAcc);
#endif

	switch (_fsmState) {
		case INTERCOM_BUDDY_LISTENING_STATE_LISTENING:
			if (_echoReplyAcc==0) {
				_fsmState = INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING;
				PLF_PRINT(PRNTGRP_DFLT, "commState==%d, buddyFSM->NotListening\n", _commState);
			}
			break;

		case INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING:
			if (_echoReplyAcc > 0) {
				_fsmState = INTERCOM_BUDDY_LISTENING_STATE_LISTENING;
				PLF_PRINT(PRNTGRP_DFLT, "commState==%d, buddyFSM->Listening\n", _commState);
			}
			break;

		default:
			break;
	}

	_echoReplyAcc = 0;
}

bool Intercom_Buddy::checkButtonAndSend(void) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	bool buttonIsPressed = _intercom_buttonsAndLedsp->buddyButtonIsPressed(_buddyIdx);

	switch (_buttonState) {
		case INTERCOM_BUDDY_BUTTON_STATE_RELEASED:
			if (buttonIsPressed && (_messageHandlerp->getMyId() != ID_UNKNOWN) && (_buddyId != ID_UNKNOWN)) {
				_intercom_outgoingp->recordRequest(RECORD_REQ_ID_BUTTON, true);
				_txCommStart();
				_sendCommStart = true;
				_buttonState = INTERCOM_BUDDY_BUTTON_STATE_PRESSED;
			}

			break;

		case INTERCOM_BUDDY_BUTTON_STATE_PRESSED:
			if (!buttonIsPressed) {
				_intercom_outgoingp->recordRequest(RECORD_REQ_ID_BUTTON, false);
				_txCommStop();
				_sendCommStop = true;
				_buttonState = INTERCOM_BUDDY_BUTTON_STATE_RELEASED;
			}

			break;

		default:
			break;
	}

	_intercom_outgoingp->run(_buddyId);

	return (_buttonState == INTERCOM_BUDDY_BUTTON_STATE_PRESSED);
}

void Intercom_Buddy::_buddyLedUpdate(void) {
	switch (_ledState) {
		case INTERCOM_BUDDY_LED_STATE_OFF:
			if (_commState == INTERCOM_BUDDY_COMM_STATE_STARTED) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BLINKING;
				_buddyLedp->blink(200, 200);
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Blinking\n", _buddyIdx);
			}
			else if (_fsmState == INTERCOM_BUDDY_LISTENING_STATE_LISTENING) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BREATHING;
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Breathing\n", _buddyIdx);
				_buddyLedp->breathe(200 /*tOn*/, 200 /*tOff*/, 1800 /*rise*/, 1800/*fall*/);
			}
			break;

		case INTERCOM_BUDDY_LED_STATE_BREATHING:
			if (_commState == INTERCOM_BUDDY_COMM_STATE_STARTED) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BLINKING;
				_buddyLedp->blink(200, 200);
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Blinking\n", _buddyIdx);
			}
			else if (_fsmState == INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING) {
				_ledState = INTERCOM_BUDDY_LED_STATE_OFF;
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Off\n", _buddyIdx);
				_buddyLedp->analogWrite(0); /*Off*/
			}
			break;

		case INTERCOM_BUDDY_LED_STATE_BLINKING:
			if (_commState == INTERCOM_BUDDY_COMM_STATE_STOPPED) {
				if (_fsmState == INTERCOM_BUDDY_LISTENING_STATE_LISTENING) {
					_ledState = INTERCOM_BUDDY_LED_STATE_BREATHING;
					PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Breathing\n", _buddyIdx);
					_buddyLedp->breathe(200 /*tOn*/, 200 /*tOff*/, 1800 /*rise*/, 1800/*fall*/);
				}
				else { /*Not listening:*/
					_ledState = INTERCOM_BUDDY_LED_STATE_OFF;
					PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Off\n", _buddyIdx);
					_buddyLedp->analogWrite(0); /*Off*/
				}
			}
			break;

		default:
			break;
	}
}

void Intercom_Buddy::_tickerHook(void) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	_buddyLedUpdate();
	_txEchoReq();
	
	if (_sendCommStart) {
		_txCommStart();
	}

	if (_sendCommStop) {
		_txCommStop();
	}

	if (++_tickCount>=4) {
		_tickCount=0;
		/*Do these every 4 ticks (2s)*/
		_txSetBuddy();
		_txWhoIsReq();
		_fsmUpdate();
	}
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
	_fsmState = INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING;
	_commState = INTERCOM_BUDDY_COMM_STATE_STOPPED;
	_ledState = INTERCOM_BUDDY_LED_STATE_OFF;
	_echoReplyAcc = 0;
	_prevMillis = 0;
	_tickCount = 0;
	_buttonState = INTERCOM_BUDDY_BUTTON_STATE_RELEASED;

	_messageHandlerp->registerHandler(WHO_IS_REPLY_T_MSG_ID, messageHandlerHelper, this, true);	
	_messageHandlerp->registerHandler(ECHO_REPLY_T_MSG_ID, messageHandlerHelper, this, true);
	_messageHandlerp->registerHandler(COMM_START_ACK_T_MSG_ID, messageHandlerHelper, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_ACK_T_MSG_ID, messageHandlerHelper, this, true);
	_messageHandlerp->registerHandler(COMM_START_T_MSG_ID, messageHandlerHelper, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_T_MSG_ID, messageHandlerHelper, this, true);

	_buddyLedp->analogWrite(0); /*Off*/

	_sendCommStart = false;
	_sendCommStop = false;

	_initialized = true;
}

int Intercom_Buddy::handleMessage(Intercom_Message& msg, int payloadSize) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	switch (msg.msgId) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _rxWhoIsRep(msg, payloadSize);

    case ECHO_REPLY_T_MSG_ID:
    	return _rxEchoRep(msg, payloadSize);

    case COMM_START_ACK_T_MSG_ID:
    	return _rxCommStartAck(msg, payloadSize);

	case COMM_STOP_ACK_T_MSG_ID:
    	return _rxCommStopAck(msg, payloadSize);

    case COMM_START_T_MSG_ID:
    	return _rxCommStart(msg, payloadSize);

	case COMM_STOP_T_MSG_ID:
    	return _rxCommStop(msg, payloadSize);

    default:
      return -(MODULE_ID+5);
  }

  return 0;
}