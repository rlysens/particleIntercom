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

int Intercom_Buddy::_rxKeepAliveResp(Intercom_Message& msg, int payloadSize) {
	keep_alive_resp_t keep_alive_resp;
	int numDecodedBytes = keep_alive_resp_t_decode(msg.data, 0, payloadSize, &keep_alive_resp);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	_buddyServerAddress = IPAddress(keep_alive_resp.redirect_addr);

	return 0;
}

int Intercom_Buddy::_setServerAddr(int key) {
	int value;
	bool valid;

	plf_registry.getInt(key, value, valid);

	if (valid) {
		IPAddress address = IPAddress(value);
		_myServerAddress = address;
		if (!_buddyServerAddress) {
			_buddyServerAddress = address;
		}

		_txSetBuddy();
	}

	return 0;
}

void Intercom_Buddy::_txSetBuddy(void) {
	int numEncodedBytes;
	uint32_t myId = _messageHandlerp->getMyId();
	set_buddy_t set_buddy;

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)
		return;

	/*Configure this buddy as the source_id for the counter object*/
	plf_assert("counter ptr NULL", _rxMsgCounterp);
	_rxMsgCounterp->source_id = _buddyId;
	_txMsgCounterp->destination_id = _buddyId;
	
	/*Set my buddy server side. Keep in mind that this code runs periodically. So if one message gets lost, no problem.*/
	set_buddy.my_id = myId;
	set_buddy.buddy_id = _buddyId; /*Note that this is 0 if buddy_id is unknown*/

	numEncodedBytes = set_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &set_buddy);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	if (_myServerAddress) {
		_messageHandlerp->send(intercom_message, SET_BUDDY_T_MSG_ID, INTERCOM_SERVER_ID, numEncodedBytes, _myServerAddress);
	}
}

int Intercom_Buddy::_setBuddy(int key) {
	int value = ID_UNKNOWN;
	bool valid;

	plf_registry.getInt(key, value, valid);
	_buddyId = value;

	_txSetBuddy();

	_sendSetBuddy=true; /*Retransmit until ack is received.*/

	return 0;
}

int Intercom_Buddy::_rxSetBuddyAck(Intercom_Message& msg, int payloadSize) {
	set_buddy_ack_t set_buddy_ack;
	int numDecodedBytes = set_buddy_ack_t_decode(msg.data, 0, payloadSize, &set_buddy_ack);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (set_buddy_ack.buddy_id != _buddyId) {
		return -(MODULE_ID+2);
	}

	_sendSetBuddy = false; /*We can stop sending now.*/
	return 0;
}

int Intercom_Buddy::_rxCommStart(Intercom_Message& msg, int payloadSize) {
	comm_start_t comm_start;
	comm_start_ack_t comm_start_ack;
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

	_messageHandlerp->send(intercom_message, COMM_START_ACK_T_MSG_ID, comm_start.source_id, numEncodedBytes, _buddyServerAddress);

	return 0;
}

int Intercom_Buddy::_rxCommStop(Intercom_Message& msg, int payloadSize) {
	comm_stop_t comm_stop;
	comm_stop_ack_t comm_stop_ack;
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

	_messageHandlerp->send(intercom_message, COMM_STOP_ACK_T_MSG_ID, comm_stop.source_id, numEncodedBytes, _buddyServerAddress);

	return 0;
}

void Intercom_Buddy::_txCommStart(void) {
	int numEncodedBytes;
	comm_start_t comm_start;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	comm_start.source_id = myId;
	comm_start.destination_id = _buddyId;

	numEncodedBytes = comm_start_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_start);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_START_T_MSG_ID, _buddyId, numEncodedBytes, _buddyServerAddress);
}

int Intercom_Buddy::_rxCommStartAck(Intercom_Message& msg, int payloadSize) {
	comm_start_ack_t comm_start_ack;
	int numDecodedBytes = comm_start_ack_t_decode(msg.data, 0, payloadSize, &comm_start_ack);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (comm_start_ack.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		_sendCommStart = false; /*Stop sending comm_start*/

	return 0;
}

void Intercom_Buddy::_txCommStop(void) {
	int numEncodedBytes;
	comm_stop_t comm_stop;
	uint32_t myId = _messageHandlerp->getMyId();

	if (myId == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	comm_stop.source_id = myId;
	comm_stop.destination_id = _buddyId;

	numEncodedBytes = comm_stop_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &comm_stop);
	plf_assert("Msg Encode Error", numEncodedBytes>=0);

	_messageHandlerp->send(intercom_message, COMM_STOP_T_MSG_ID, _buddyId, numEncodedBytes, _buddyServerAddress);
}

int Intercom_Buddy::_rxCommStopAck(Intercom_Message& msg, int payloadSize) {
	comm_stop_ack_t comm_stop_ack;
	int numDecodedBytes = comm_stop_ack_t_decode(msg.data, 0, payloadSize, &comm_stop_ack);

	if (numDecodedBytes < 0)
		return -(MODULE_ID+1);

	if (comm_stop_ack.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		_sendCommStop = false; /*Stop sending comm_stop*/

	return 0;
}

void Intercom_Buddy::_txKeepAlive(void) {
	int numEncodedBytes;
	keep_alive_t keep_alive;
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

	_messageHandlerp->send(intercom_message, KEEP_ALIVE_T_MSG_ID, _buddyId, numEncodedBytes, _buddyServerAddress);
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
	const char *requestTypeStrings[] = {"Button", "Incoming Comm"};
	int ii;

	plf_assert("requestType out of range", requestType < INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES);

  	_outgoingCommRequests[requestType] = enable;

	switch (_outgoingCommFsmState) {
		case INTERCOM_BUDDY_OUTGOING_COMM_IDLE:
			for (ii=0; ii<INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES; ++ii) {
				if (_outgoingCommRequests[ii]) {
				  _outgoingCommFsmState = INTERCOM_BUDDY_OUTGOING_COMM_REQUESTED;
				  PLF_PRINT(PRNTGRP_DFLT, "Intercom_Buddy %d Outgoing Idle->Comm Requested by %s\n", _buddyIdx, 
				  	requestTypeStrings[ii]);
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

	if (_sendSetBuddy) {
		_txSetBuddy();
	}

	if (++_tickCount>=10) {
		_tickCount=0;
		/*Do these every 10 ticks (5s)*/
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
	
	_messageHandlerp->registerHandler(COMM_START_ACK_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_ACK_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_START_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(COMM_STOP_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(KEEP_ALIVE_RESP_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);
	_messageHandlerp->registerHandler(SET_BUDDY_ACK_T_MSG_ID, &Intercom_Buddy::handleMessage, this, true);


	PLF_REGISTRY_REGISTER_HANDLER(regKey_buddyId[_buddyIdx], &Intercom_Buddy::_setBuddy, this);
	PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SRVR_ADDR, &Intercom_Buddy::_setServerAddr, this);

	_buddyLedp->analogWrite(0); /*Off*/

	_sendCommStart = false;
	_sendCommStop = false;
	_sendSetBuddy = false;

	dataDump.registerFunction(String::format("Buddy%d", _buddyIdx), &Intercom_Buddy::_dataDump, this);

	_initialized = true;
}

int Intercom_Buddy::handleMessage(Intercom_Message& msg, int payloadSize) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	switch (msg.msgId) {
    case COMM_START_ACK_T_MSG_ID:
    	return _rxCommStartAck(msg, payloadSize);

	case COMM_STOP_ACK_T_MSG_ID:
    	return _rxCommStopAck(msg, payloadSize);

    case COMM_START_T_MSG_ID:
    	return _rxCommStart(msg, payloadSize);

	case COMM_STOP_T_MSG_ID:
    	return _rxCommStop(msg, payloadSize);

	case KEEP_ALIVE_RESP_T_MSG_ID:
    	return _rxKeepAliveResp(msg, payloadSize);

    case SET_BUDDY_ACK_T_MSG_ID:
    	return _rxSetBuddyAck(msg, payloadSize);

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