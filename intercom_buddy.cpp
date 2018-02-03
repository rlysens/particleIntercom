#include "intercom_buddy.h"
#include "messages.h"
#include "plf_utils.h"
#include "vs1063a_codec.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 100

#define INTERCOM_BUDDY_LISTENING_STATE_LISTENING 1 /*Buddy is listening*/
#define INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING 0 /*Buddy is not listening*/

#define INTERCOM_BUDDY_BUTTON_STATE_RELEASED 0
#define INTERCOM_BUDDY_BUTTON_STATE_PRESSED 1

#define INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED 0
#define INTERCOM_BUDDY_INCOMING_COMM_STATE_STOPPED 1
#define INTERCOM_BUDDY_INCOMING_COMM_STATE_SUSPENDED 2
#define INTERCOM_BUDDY_NUM_INCOMING_COMM_STATES 3

#define INTERCOM_BUDDY_OUTGOING_COMM_IDLE 0
#define INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED 1

#define INTERCOM_BUDDY_LED_STATE_OFF 0
#define INTERCOM_BUDDY_LED_STATE_BREATHING 1
#define INTERCOM_BUDDY_LED_STATE_BLINKING 2
#define INTERCOM_BUDDY_NUM_LED_STATES (INTERCOM_BUDDY_LED_STATE_BLINKING+1)

#define INTERCOM_BUDDY_TICK_INTER_MS 500

static const int regKey_buddyId[NUM_BUDDIES] = {REG_KEY_BUDDY_0_ID, REG_KEY_BUDDY_1_ID, REG_KEY_BUDDY_2_ID};
static const int regKey_buddyName[NUM_BUDDIES] = {REG_KEY_BUDDY_0_NAME, REG_KEY_BUDDY_1_NAME, REG_KEY_BUDDY_2_NAME};

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

	_messageHandlerp->send(intercom_message, SET_BUDDY_T_MSG_ID, INTERCOM_SERVER_ID, numEncodedBytes, true);
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

	_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_INCOMING_COMM, true);

	_incomingCommState = INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED;

	comm_start_ack.source_id = myId;
	comm_start_ack.destination_id = comm_start.source_id;

	numEncodedBytes = comm_start_ack_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_start_ack);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_START_ACK_T_MSG_ID, comm_start.source_id, numEncodedBytes, true);

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

	_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_INCOMING_COMM, false);

	_incomingCommState = INTERCOM_BUDDY_INCOMING_COMM_STATE_STOPPED;

	comm_stop_ack.source_id = myId;
	comm_stop_ack.destination_id = comm_stop.source_id;

	numEncodedBytes = comm_stop_ack_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_stop_ack);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_STOP_ACK_T_MSG_ID, comm_stop.source_id, numEncodedBytes, true);

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

	_messageHandlerp->send(intercom_message, COMM_START_T_MSG_ID, _buddyId, numEncodedBytes, true);
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

	_messageHandlerp->send(intercom_message, COMM_STOP_T_MSG_ID, _buddyId, numEncodedBytes, true);
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

void Intercom_Buddy::_txKeepAlive(void) {
	int numEncodedBytes;
	static keep_alive_t keep_alive;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	plf_assert("NULL txMsgCounterp", _txMsgCounterp);

	/*Only send keep-alive when there's been no incoming communication*/
	if (_txMsgCounterp->txMsgCounter != _prevTxMsgCounter) {
		_prevTxMsgCounter = _txMsgCounterp->txMsgCounter;
		return;
	}

	keep_alive.source_id = myId;
	keep_alive.destination_id = _buddyId;

	numEncodedBytes = keep_alive_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &keep_alive);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, KEEP_ALIVE_T_MSG_ID, _buddyId, numEncodedBytes, true);
}

void Intercom_Buddy::_txWhoIsReq(void) {
	int numEncodedBytes;
	static who_is_t who_is;
	String buddyName;
	bool buddyNameIsSet;

	plf_registry.get(regKey_buddyName[_buddyIdx], buddyName, buddyNameIsSet);
	if (!buddyNameIsSet)
		return;

	buddyName.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	numEncodedBytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, WHO_IS_T_MSG_ID, INTERCOM_SERVER_ID, numEncodedBytes, true);
}

int Intercom_Buddy::_rxWhoIsRep(Intercom_Message& msg, int payloadSize) {
	static who_is_reply_t who_is_reply;
	String buddyName;
	String buddyId_s;
	bool buddyNameIsSet;
	int numDecodedBytes = who_is_reply_t_decode(msg.data, 0, payloadSize, &who_is_reply);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+2);

	plf_registry.get(regKey_buddyName[_buddyIdx], buddyName, buddyNameIsSet);
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
	plf_registry.set(regKey_buddyId[_buddyIdx], buddyId_s, true /*validity*/, false /*persistency*/);
	_buddyId = who_is_reply.id;

	/*Configure this buddy as the source_id for the counter object*/
	plf_assert("counter ptr NULL", _rxMsgCounterp);
	_rxMsgCounterp->source_id = _buddyId;
	_txMsgCounterp->destination_id = _buddyId;

	return 0;
}

void Intercom_Buddy::_listeningStateUpdate(void) {
	plf_assert("counter ptr NULL", _rxMsgCounterp);

	switch (_listeningState) {
		case INTERCOM_BUDDY_LISTENING_STATE_LISTENING:
			if (_rxMsgCounterp->rxMsgCounter == 0) {
				_listeningState = INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING;
				PLF_PRINT(PRNTGRP_DFLT, "commState==%d, buddyFSM->NotListening\n", (int)_incomingCommState);
			}
			break;

		case INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING:
			if (_rxMsgCounterp->rxMsgCounter > 0) {
				_listeningState = INTERCOM_BUDDY_LISTENING_STATE_LISTENING;
				PLF_PRINT(PRNTGRP_DFLT, "commState==%d, buddyFSM->Listening\n", (int)_incomingCommState);
			}
			break;

		default:
			break;
	}

	_rxMsgCounterp->rxMsgCounter = 0;
}

bool Intercom_Buddy::checkButton(void) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	bool buttonIsPressed = _intercom_buttonsAndLedsp->buddyButtonIsPressed(_buddyIdx);

	switch (_buttonState) {
		case INTERCOM_BUDDY_BUTTON_STATE_RELEASED:
			if (buttonIsPressed && (_messageHandlerp->getMyId() != ID_UNKNOWN) && (_buddyId != ID_UNKNOWN)) {
				_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_BUTTON, true);
				_txCommStart();
				_sendCommStart = true;
				_sendCommStop = false;
				_buttonState = INTERCOM_BUDDY_BUTTON_STATE_PRESSED;
			}

			break;

		case INTERCOM_BUDDY_BUTTON_STATE_PRESSED:
			if (!buttonIsPressed) {
				_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_BUTTON, false);
				_txCommStop();
				_sendCommStop = true;
				_sendCommStart = false;
				_buttonState = INTERCOM_BUDDY_BUTTON_STATE_RELEASED;
			}

			break;

		default:
			break;
	}

	return (_buttonState == INTERCOM_BUDDY_BUTTON_STATE_PRESSED);
}

void Intercom_Buddy::_buddyLedUpdate(void) {
	switch (_ledState) {
		case INTERCOM_BUDDY_LED_STATE_OFF:
			if (_incomingCommState == INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BLINKING;
				_buddyLedp->blink(200, 200);
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Blinking\n", _buddyIdx);
			}
			else if (_listeningState == INTERCOM_BUDDY_LISTENING_STATE_LISTENING) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BREATHING;
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Breathing\n", _buddyIdx);
				_buddyLedp->breathe(200 /*tOn*/, 200 /*tOff*/, 1800 /*rise*/, 1800/*fall*/);
			}
			break;

		case INTERCOM_BUDDY_LED_STATE_BREATHING:
			if (_incomingCommState == INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED) {
				_ledState = INTERCOM_BUDDY_LED_STATE_BLINKING;
				_buddyLedp->blink(200, 200);
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Blinking\n", _buddyIdx);
			}
			else if (_listeningState == INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING) {
				_ledState = INTERCOM_BUDDY_LED_STATE_OFF;
				PLF_PRINT(PRNTGRP_DFLT, "Buddy %d LED state -> Off\n", _buddyIdx);
				_buddyLedp->analogWrite(0); /*Off*/
			}
			break;

		case INTERCOM_BUDDY_LED_STATE_BLINKING:
			if (_incomingCommState != INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED) {
				if (_listeningState == INTERCOM_BUDDY_LISTENING_STATE_LISTENING) {
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

void Intercom_Buddy::_incomingCommStateSuspendCheck(void) {
	switch (_incomingCommState) {
		case INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED:
			if (!_intercom_incomingp->isSenderActive(_buddyId)) {
				_incomingCommState = INTERCOM_BUDDY_INCOMING_COMM_STATE_SUSPENDED;
				_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_INCOMING_COMM, false);
			}
			break;

		case INTERCOM_BUDDY_INCOMING_COMM_STATE_SUSPENDED:
			if (_intercom_incomingp->isSenderActive(_buddyId)) {
				_incomingCommState = INTERCOM_BUDDY_INCOMING_COMM_STATE_STARTED;
				_outgoingCommRequest(INTERCOM_BUDDY_OUTGOING_REQ_TYPE_INCOMING_COMM, true);
			}
			break;

		default:
			break;
	}
}

bool Intercom_Buddy::outgoingCommRequested(void) {
	return _outgoingCommFsmState==INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED;
}

void Intercom_Buddy::_outgoingCommRequest(unsigned requestType, bool enable) {
	int ii;

	plf_assert("requestType out of range", requestType < INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES);

  	_outgoingCommRequests[requestType] = enable;

	switch (_outgoingCommFsmState) {
		case INTERCOM_BUDDY_OUTGOING_COMM_IDLE:
			for (ii=0; ii<INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES; ++ii) {
				if (_outgoingCommRequests[ii]) {
				  _outgoingCommFsmState = INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED;
				  PLF_PRINT(PRNTGRP_DFLT, "Intercom_Buddy %d Outgoing Idle->Comm Requested by %d\n", _buddyIdx, ii);
				}
			}
			break;

		case INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED:
			{
			    bool outgoingCommRequested = false;

			    for (ii=0; ii<INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES; ++ii) {
			      if (_outgoingCommRequests[ii]) {
			        outgoingCommRequested = true;
			        break;
			      }
			    }

			    if (!outgoingCommRequested) {
			      _outgoingCommFsmState = INTERCOM_BUDDY_OUTGOING_COMM_IDLE;
			      PLF_PRINT(PRNTGRP_DFLT, "Intercom_Buddy %d Outgoing Comm Requested->Idle\n", _buddyIdx);
			    }
			}
		  	break;

		default:
		  	break;
	}
}

void Intercom_Buddy::_tickerHook(void) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	_incomingCommStateSuspendCheck();
	_buddyLedUpdate();
	_txKeepAlive();

	if (_sendCommStart) {
		_txCommStart();
	}

	if (_sendCommStop) {
		_txCommStop();
	}

	if (++_tickCount>=10) {
		_tickCount=0;
		/*Do these every 10 ticks (5s)*/
		_txSetBuddy();
		_txWhoIsReq();
		_listeningStateUpdate();
	}
}

Intercom_Buddy::Intercom_Buddy() : Plf_TickerBase(INTERCOM_BUDDY_TICK_INTER_MS), _initialized(false) {
}

void Intercom_Buddy::init(Intercom_Incoming* intercom_incomingp,
	Intercom_MessageHandler* messageHandlerp, 
	Intercom_ButtonsAndLeds* intercom_buttonsAndLedsp, int buddyIdx) {

	plf_assert("NULL ptr in IntercomBuddy::init", intercom_incomingp);
	plf_assert("NULL ptr in IntercomBuddy::init", messageHandlerp);
	plf_assert("NULL ptr in IntercomBuddy::init", intercom_buttonsAndLedsp);
	plf_assert("BuddyIdx out of range", buddyIdx<NUM_BUDDIES);

	_intercom_incomingp = intercom_incomingp;
	_messageHandlerp = messageHandlerp;
	_intercom_buttonsAndLedsp = intercom_buttonsAndLedsp;
	_buddyLedp = &(intercom_buttonsAndLedsp->getBuddyLed(buddyIdx));
	_txMsgCounterp = _messageHandlerp->allocTxCounter();
	_rxMsgCounterp = _messageHandlerp->allocRxCounter();
	_prevTxMsgCounter = 0;

	plf_assert("NULL msgCounterp", _rxMsgCounterp);

	_buddyIdx = buddyIdx;
	_buddyId = ID_UNKNOWN;
	_listeningState = INTERCOM_BUDDY_LISTENING_STATE_NOT_LISTENING;
	_incomingCommState = INTERCOM_BUDDY_INCOMING_COMM_STATE_STOPPED;
	_ledState = INTERCOM_BUDDY_LED_STATE_OFF;
	_outgoingCommFsmState = INTERCOM_BUDDY_OUTGOING_COMM_IDLE;
	_prevMillis = 0;
	_tickCount = 0;
	_buttonState = INTERCOM_BUDDY_BUTTON_STATE_RELEASED;

	memset(_outgoingCommRequests, 0, sizeof(_outgoingCommRequests));

	_messageHandlerp->registerHandler(WHO_IS_REPLY_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);	
	_messageHandlerp->registerHandler(COMM_START_ACK_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_ACK_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_START_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);

	_buddyLedp->analogWrite(0); /*Off*/

	_sendCommStart = false;
	_sendCommStop = false;

	dataDump.registerFunction(String::format("Buddy%d", _buddyIdx), &Intercom_Buddy::_dataDump, this);

	_initialized = true;
}

int Intercom_Buddy::handleMessage(Intercom_Message& msg, int payloadSize) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	switch (msg.msgId) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _rxWhoIsRep(msg, payloadSize);

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

void Intercom_Buddy::_dataDump(void) {
	const char* ledStateStrings[INTERCOM_BUDDY_NUM_LED_STATES] = {"Off", "Breathing", "Blinking"};
	const char* incomingCommStateStrings[INTERCOM_BUDDY_NUM_INCOMING_COMM_STATES] = {"Started", "Stopped", "Suspended"};

	PLF_PRINT(PRNTGRP_DFLT, "BuddyId: %d", (int)_buddyId);
	PLF_PRINT(PRNTGRP_DFLT, "ListeningState: %s", _listeningState==INTERCOM_BUDDY_LISTENING_STATE_LISTENING ? 
		"Listening" : "Not Listening");
	PLF_PRINT(PRNTGRP_DFLT, "IncomingCommState: %s", incomingCommStateStrings[_incomingCommState]);
	PLF_PRINT(PRNTGRP_DFLT, "LedState: %s", ledStateStrings[_ledState]);
	PLF_PRINT(PRNTGRP_DFLT, "ButtonState: %s", _buttonState==INTERCOM_BUDDY_BUTTON_STATE_RELEASED ? "Released" : "Pressed");
	PLF_PRINT(PRNTGRP_DFLT, "OutgoingCommRequested: %d", (int)(_outgoingCommFsmState==INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED));
}