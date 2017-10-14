#include "intercom_controller.h"
#include "messages.h"

#define INTERCOM_CONTROLLER_TICK_INTER_MS 2000
#define BUDDY_LISTENING_LED D7

#define FSM_ENCRYPTION_DISABLED 0
#define FSM_ENCRYPTION_ENABLED 1

static Intercom_Message intercom_message;

static int message_handler_helper(Intercom_Message &msg, 
  int payload_size, void *ctxt) {
	Intercom_Controller *intercom_controllerp = (Intercom_Controller*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_controllerp);

  return intercom_controllerp->handle_message(msg, payload_size);
}

void Intercom_Controller::_i_am(void) {
	int num_encoded_bytes;
	static Intercom_Message intercom_message;
	static i_am_t i_am;
	bool my_name_is_set;
	String my_name;

	_registry.get(REG_KEY_MY_NAME, my_name, my_name_is_set);
	if (!my_name_is_set)
		return;

	my_name.getBytes((unsigned char*)i_am.name, sizeof(i_am.name));

	num_encoded_bytes = i_am_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &i_am);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, I_AM_T_MSG_ID, num_encoded_bytes, false);
}

void Intercom_Controller::_buddy_listening_led(bool on) {
	digitalWrite(BUDDY_LISTENING_LED, on ? HIGH : LOW); 
}

void Intercom_Controller::_tx_echo_req(void) {
	int num_encoded_bytes;
	static echo_request_t echo_request;
	bool buddy_id_is_set;
	String buddy_id_s;

	_buddy_listening_led(false); /*led off. Echo response will turn it back on.*/

	if (!_my_id_is_known)
		return;

	_registry.get(REG_KEY_BUDDY_ID, buddy_id_s, buddy_id_is_set);
	if (!buddy_id_is_set)	
		return;

	echo_request.source_id = _my_id;
	echo_request.destination_id = buddy_id_s.toInt();

	num_encoded_bytes = echo_request_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_request);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, ECHO_REQUEST_T_MSG_ID, num_encoded_bytes, true);
}

void Intercom_Controller::_whois(void) {
	int num_encoded_bytes;
	static who_is_t who_is;
	String buddy_name;
	bool buddy_name_is_set;

	_registry.get(REG_KEY_BUDDY_NAME, buddy_name, buddy_name_is_set);
	if (!buddy_name_is_set)
		return;

	buddy_name.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	num_encoded_bytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, WHO_IS_T_MSG_ID, num_encoded_bytes, true);
}

int Intercom_Controller::_rx_echo_request(Intercom_Message& msg, int payload_size) {
	static echo_request_t echo_request;
	static echo_reply_t echo_reply;
	int num_encoded_bytes;
	int num_decoded_bytes = echo_request_t_decode(msg.data, 0, payload_size, &echo_request);

	if (num_decoded_bytes < 0)
		return -1;

	if (!_my_id_is_known)
		return -1;

	echo_reply.source_id = _my_id;
	echo_reply.destination_id = echo_request.source_id;

	num_encoded_bytes = echo_reply_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_reply);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, ECHO_REPLY_T_MSG_ID, num_encoded_bytes, true);

	return 0;
}

int Intercom_Controller::_whois_reply(Intercom_Message& msg, int payload_size) {
	static who_is_reply_t who_is_reply;
	static set_buddy_t set_buddy;
	String buddy_name;
	String buddy_id_s;
	int buddy_id_i;
	bool buddy_id_is_known;
	bool buddy_name_is_set;
	int num_encoded_bytes;
	int num_decoded_bytes = who_is_reply_t_decode(msg.data, 0, payload_size, &who_is_reply);

	if (num_decoded_bytes < 0)
		return -1;

	_registry.get(REG_KEY_BUDDY_NAME, buddy_name, buddy_name_is_set);
	if (!buddy_name_is_set)
		return -2;

	/*zero terminate*/
	who_is_reply.name[sizeof(who_is_reply.name)-1] = 0;

	if (!String((const char*)who_is_reply.name).equals(buddy_name))
		return -3;

	_registry.get(REG_KEY_BUDDY_ID, buddy_id_s, buddy_id_is_known);

	if (!buddy_id_is_known) {
		PLF_PRINT(PRNTGRP_DFLT, "Buddy id received %d\n", (int)who_is_reply.id);
	}


	/*Put a string version of the buddy_id in the registry*/
	buddy_id_s = String(who_is_reply.id);
	_registry.set(REG_KEY_BUDDY_ID, buddy_id_s, true /*validity*/, false /*persistency*/);

	if (_my_id_is_known) {
		/*Set my buddy server side. Keep in mind that this code runs periodically. So if one message gets lost, no problem.*/
		set_buddy.my_id = _my_id;
		set_buddy.buddy_id = who_is_reply.id;

		num_encoded_bytes = set_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &set_buddy);
		plf_assert("Msg Encode Error", num_encoded_bytes>=0);

		_message_handler.send(intercom_message, SET_BUDDY_T_MSG_ID, num_encoded_bytes, true);
	}

	_intercom_outgoing.set_destination_id(who_is_reply.id);
	return 0;
}

int Intercom_Controller::_i_am_reply(Intercom_Message& msg, int payload_size) {
	i_am_reply_t i_am_reply;
	bool my_name_is_set;
	String my_name;
	int num_decoded_bytes = i_am_reply_t_decode(msg.data, 0, payload_size, &i_am_reply);

	if (num_decoded_bytes < 0) {
		return -1;
	}

	_registry.get(REG_KEY_MY_NAME, my_name, my_name_is_set);
	if (!my_name_is_set) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply received while my_name not set\n");
		return -3;
	}

	if (!String((const char*)i_am_reply.name).equals(my_name)) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply string mismatch\n");
		Serial.println(String((const char*)i_am_reply.name));
		Serial.println(my_name);
		return -2;
	}

	if (!_my_id_is_known) {
		PLF_PRINT(PRNTGRP_DFLT, "My id received %d\n", (int)i_am_reply.id);
	}

	_my_id_is_known = true;
	_my_id = i_am_reply.id;

	_intercom_outgoing.set_source_id(i_am_reply.id);
	_message_handler.set_source_id(i_am_reply.id);

	_message_handler.set_encryption_key((uint8_t*)(i_am_reply.key));
	_fsm_state = FSM_ENCRYPTION_ENABLED;

	return 0;
}

int Intercom_Controller::_echo_reply(Intercom_Message& msg, int payload_size) {
	_buddy_listening_led(true); /*Turn LED on*/

	return 0;
}

int Intercom_Controller::handle_message(Intercom_Message& msg, int payload_size) {
	switch (msg.msg_id) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _whois_reply(msg, payload_size);

    case I_AM_REPLY_T_MSG_ID:
    	return _i_am_reply(msg, payload_size);

    case ECHO_REPLY_T_MSG_ID:
    	return _echo_reply(msg, payload_size);

    case ECHO_REQUEST_T_MSG_ID:
    	return _rx_echo_request(msg, payload_size);

    default:
      return -1;
  }

  return 0;
}

void Intercom_Controller::tick(void) {
	unsigned long cur_millis = millis();
	unsigned long millis_delta;

	if (cur_millis < _prev_millis) {
		millis_delta = (~0UL) - _prev_millis + cur_millis;
	}
	else {
		millis_delta = cur_millis - _prev_millis;
	}

	if (millis_delta > INTERCOM_CONTROLLER_TICK_INTER_MS) {
		_prev_millis = cur_millis;

		if (_fsm_state == FSM_ENCRYPTION_DISABLED) {
			_i_am();
		}
		else { /*FSM_ENCRYPTION_ENABLED*/
			_whois();
			_tx_echo_req();
		}
	}
}

Intercom_Controller::Intercom_Controller(Message_Handler& message_handler, 
	Intercom_Outgoing& intercom_outgoing, PlfRegistry &registry) : 
	_message_handler(message_handler), _intercom_outgoing(intercom_outgoing),
	_fsm_state(FSM_ENCRYPTION_DISABLED), _my_id_is_known(false), _prev_millis(0),
 	_registry(registry) {
	_message_handler.register_handler(WHO_IS_REPLY_T_MSG_ID, message_handler_helper, this, true);
	_message_handler.register_handler(I_AM_REPLY_T_MSG_ID, message_handler_helper, this, false);
	_message_handler.register_handler(ECHO_REPLY_T_MSG_ID, message_handler_helper, this, true);
	_message_handler.register_handler(ECHO_REQUEST_T_MSG_ID, message_handler_helper, this, true);

	pinMode(BUDDY_LISTENING_LED, OUTPUT);
}
