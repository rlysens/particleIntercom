#include "intercom_buddy.h"
#include "messages.h"
#include "plf_utils.h"

#define INTERCOM_BUDDY_FSM_STATE_LISTENING 1 /*Buddy is listening*/
#define INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING 0 /*Buddy is not listening*/

#define INTERCOM_BUDDY_TICK_INTER_MS 2000

static const int reg_key_buddy_id[NUM_BUDDIES] = {REG_KEY_BUDDY_0_ID, REG_KEY_BUDDY_1_ID, REG_KEY_BUDDY_2_ID};
static const int reg_key_buddy_name[NUM_BUDDIES] = {REG_KEY_BUDDY_0_NAME, REG_KEY_BUDDY_1_NAME, REG_KEY_BUDDY_2_NAME};

static int message_handler_helper(Intercom_Message &msg, 
  int payload_size, void *ctxt) {
	IntercomBuddy *intercom_buddyp = (IntercomBuddy*)ctxt;

	plf_assert("NULL ctxt ptr", intercom_buddyp);

 	return intercom_buddyp->handle_message(msg, payload_size);
}

void IntercomBuddy::_txSetBuddy(void) {
	int num_encoded_bytes;
	uint32_t my_id = _message_handlerp->getMyId();
	static set_buddy_t set_buddy;

	if (my_id == ID_UNKNOWN)
		return;

	/*Set my buddy server side. Keep in mind that this code runs periodically. So if one message gets lost, no problem.*/
	set_buddy.my_id = my_id;
	set_buddy.buddy_id = _buddyId; /*Note that this is 0 if buddy_id is unknown*/

	num_encoded_bytes = set_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &set_buddy);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handlerp->send(intercom_message, SET_BUDDY_T_MSG_ID, num_encoded_bytes, true);
}

void IntercomBuddy::_txEchoReq(void) {
	int num_encoded_bytes;
	static echo_request_t echo_request;
	uint32_t my_id = _message_handlerp->getMyId();

	if (my_id == ID_UNKNOWN)
		return;

	if (_buddyId == ID_UNKNOWN)	
		return;

	echo_request.source_id = my_id;
	echo_request.destination_id = _buddyId;

	num_encoded_bytes = echo_request_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_request);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handlerp->send(intercom_message, ECHO_REQUEST_T_MSG_ID, num_encoded_bytes, true);
}

int IntercomBuddy::_rxEchoRep(Intercom_Message& msg, int payload_size) {
	static echo_reply_t echo_reply;
	int num_decoded_bytes = echo_reply_t_decode(msg.data, 0, payload_size, &echo_reply);

	if (num_decoded_bytes < 0)
		return -1;

	if (echo_reply.source_id == (int32_t)_buddyId) /*Did this buddy send it?*/
		++_echo_reply_acc;

	return 0;
}

void IntercomBuddy::_txWhoIsReq(void) {
	int num_encoded_bytes;
	static who_is_t who_is;
	String buddy_name;
	bool buddy_name_is_set;

	_registryp->get(reg_key_buddy_name[_buddyIdx], buddy_name, buddy_name_is_set);
	if (!buddy_name_is_set)
		return;

	buddy_name.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	num_encoded_bytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handlerp->send(intercom_message, WHO_IS_T_MSG_ID, num_encoded_bytes, true);
}

int IntercomBuddy::_rxWhoIsRep(Intercom_Message& msg, int payload_size) {
	static who_is_reply_t who_is_reply;
	String buddy_name;
	String buddy_id_s;
	bool buddy_name_is_set;
	int num_decoded_bytes = who_is_reply_t_decode(msg.data, 0, payload_size, &who_is_reply);

	if (num_decoded_bytes < 0)
		return -1;

	_registryp->get(reg_key_buddy_name[_buddyIdx], buddy_name, buddy_name_is_set);
	if (!buddy_name_is_set) {
		return -2;
	}

	/*zero terminate*/
	who_is_reply.name[sizeof(who_is_reply.name)-1] = 0;

	if (!String((const char*)who_is_reply.name).equals(buddy_name))
		return -3;

	if (_buddyId == ID_UNKNOWN) {
		PLF_PRINT(PRNTGRP_DFLT, "Buddy id received %d, buddyIdx %d\n", (int)who_is_reply.id, _buddyIdx);
	}

	/*Put a string version of the buddy_id in the registry*/
	buddy_id_s = String(who_is_reply.id);
	_registryp->set(reg_key_buddy_id[_buddyIdx], buddy_id_s, true /*validity*/, false /*persistency*/);
	_buddyId = who_is_reply.id;
	
	return 0;
}

void IntercomBuddy::_fsm(void) {
	if (_fsm_state == INTERCOM_BUDDY_FSM_STATE_LISTENING) {
		if (_echo_reply_acc == 0) {
			_fsm_state = INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING;
			_buddyLedp->analogWrite(0); /*Off*/
			PLF_PRINT(PRNTGRP_DFLT, "buddyFSM->NotListening\n");
		}
	}
	else { /*Not Listening state:*/
		if (_echo_reply_acc > 0) {
			_fsm_state = INTERCOM_BUDDY_FSM_STATE_LISTENING;
			_buddyLedp->breathe(200 /*tOn*/, 200 /*tOff*/, 1800 /*rise*/, 1800/*fall*/);
			PLF_PRINT(PRNTGRP_DFLT, "buddyFSM->Listening\n");
		}
	}

	_echo_reply_acc = 0;
}

bool IntercomBuddy::checkButtonAndSend(void) {
	bool buttonIsPressed;

	plf_assert("IntercomBuddy not initialized", _initialized);
	
	buttonIsPressed = _intercomButtonsAndLedsp->buddyButtonIsPressed(_buddyIdx);

	if (buttonIsPressed) {
  		if ((_message_handlerp->getMyId() != ID_UNKNOWN) && (_buddyId != ID_UNKNOWN)) {
  			_intercom_outgoingp->transfer(_buddyId);
  		}
  	}

  	return buttonIsPressed;
}

IntercomBuddy::IntercomBuddy() : _initialized(false) {
}

void IntercomBuddy::init(Intercom_Outgoing* intercom_outgoingp, Message_Handler* message_handlerp, PlfRegistry* registryp, 
	IntercomButtonsAndLeds* intercomButtonsAndLedsp, int buddyIdx) {

	plf_assert("NULL ptr in IntercomBuddy::init", intercom_outgoingp);
	plf_assert("NULL ptr in IntercomBuddy::init", message_handlerp);
	plf_assert("NULL ptr in IntercomBuddy::init", registryp);
	plf_assert("NULL ptr in IntercomBuddy::init", intercomButtonsAndLedsp);
	plf_assert("BuddyIdx out of range", buddyIdx<NUM_BUDDIES);

	_intercom_outgoingp = intercom_outgoingp;
	_message_handlerp = message_handlerp;
	_registryp = registryp;
	_intercomButtonsAndLedsp = intercomButtonsAndLedsp;
	_buddyLedp = &(intercomButtonsAndLedsp->getBuddyLed(buddyIdx));
	_buddyIdx = buddyIdx;
	_buddyId = ID_UNKNOWN;
	_fsm_state = INTERCOM_BUDDY_FSM_STATE_NOT_LISTENING;
	_echo_reply_acc = 0;
	_prev_millis = 0;

	_message_handlerp->register_handler(WHO_IS_REPLY_T_MSG_ID, message_handler_helper, this, true);	
	_message_handlerp->register_handler(ECHO_REPLY_T_MSG_ID, message_handler_helper, this, true);

	_buddyLedp->analogWrite(0); /*Off*/

	_initialized = true;
}

int IntercomBuddy::handle_message(Intercom_Message& msg, int payload_size) {
	plf_assert("IntercomBuddy not initialized", _initialized);

	switch (msg.msg_id) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _rxWhoIsRep(msg, payload_size);

    case ECHO_REPLY_T_MSG_ID:
    	return _rxEchoRep(msg, payload_size);

    default:
      return -1;
  }

  return 0;
}

void IntercomBuddy::tick(void) {
	unsigned long cur_millis = millis();
	unsigned long millis_delta;

	plf_assert("IntercomBuddy not initialized", _initialized);

	if (cur_millis < _prev_millis) {
		millis_delta = (~0UL) - _prev_millis + cur_millis;
	}
	else {
		millis_delta = cur_millis - _prev_millis;
	}

	if (millis_delta > INTERCOM_BUDDY_TICK_INTER_MS) {
		_prev_millis = cur_millis;

		_txSetBuddy();
		_txWhoIsReq();
		_txEchoReq();
		_fsm();
	}
}