#include "intercom_controller.h"
#include "messages.h"
#include "plf_utils.h"

#define INTERCOM_CONTROLLER_TICK_INTER_MS 2000

#define INTERCOM_CONTROLLER_FSM_STATE_RESTARTED 0
#define INTERCOM_CONTROLLER_FSM_STATE_STEADY 1

static int message_handler_helper(Intercom_Message &msg, 
  int payload_size, void *ctxt) {
	Intercom_Controller *intercom_controllerp = (Intercom_Controller*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_controllerp);

  return intercom_controllerp->handle_message(msg, payload_size);
}

void Intercom_Controller::_i_am(void) {
	int num_encoded_bytes;
	static i_am_t i_am;
	bool my_name_is_set;
	String my_name;

	_registry.get(REG_KEY_MY_NAME, my_name, my_name_is_set);
	if (!my_name_is_set)
		return;

	my_name.getBytes((unsigned char*)i_am.name, sizeof(i_am.name));

	i_am.restarted = (_fsm_state == INTERCOM_CONTROLLER_FSM_STATE_RESTARTED) ? 1 : 0;

	num_encoded_bytes = i_am_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &i_am);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, I_AM_T_MSG_ID, num_encoded_bytes, false);
}

int Intercom_Controller::_rx_echo_request(Intercom_Message& msg, int payload_size) {
	static echo_request_t echo_request;
	static echo_reply_t echo_reply;
	int num_encoded_bytes;
	int num_decoded_bytes = echo_request_t_decode(msg.data, 0, payload_size, &echo_request);
	uint32_t my_id = _message_handler.getMyId();

	if (num_decoded_bytes < 0)
		return -1;

	if (my_id==ID_UNKNOWN)
		return -1;

	echo_reply.source_id = my_id;
	echo_reply.destination_id = echo_request.source_id;

	num_encoded_bytes = echo_reply_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &echo_reply);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	_message_handler.send(intercom_message, ECHO_REPLY_T_MSG_ID, num_encoded_bytes, true);

	return 0;
}

int Intercom_Controller::_i_am_reply(Intercom_Message& msg, int payload_size) {
	i_am_reply_t i_am_reply;
	bool my_name_is_set;
	String my_name;
	int num_decoded_bytes = i_am_reply_t_decode(msg.data, 0, payload_size, &i_am_reply);

	if (num_decoded_bytes < 0) {
		return -10;
	}

	_registry.get(REG_KEY_MY_NAME, my_name, my_name_is_set);
	if (!my_name_is_set) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply received while my_name not set\n");
		return -11;
	}

	if (!String((const char*)i_am_reply.name).equals(my_name)) {
		PLF_PRINT(PRNTGRP_DFLT, "i_am_reply string mismatch\n");
		Serial.println(String((const char*)i_am_reply.name));
		Serial.println(my_name);
		return -12;
	}

	if (_message_handler.getMyId()==ID_UNKNOWN) {
		PLF_PRINT(PRNTGRP_DFLT, "My id received %d\n", (int)i_am_reply.id);
	}

	_message_handler.setMyId(i_am_reply.id);

	_fsm_state = INTERCOM_CONTROLLER_FSM_STATE_STEADY;
	
	return 0;
}

int Intercom_Controller::handle_message(Intercom_Message& msg, int payload_size) {
	switch (msg.msg_id) {

    case I_AM_REPLY_T_MSG_ID:
    	return _i_am_reply(msg, payload_size);

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

		_i_am();
	}
}

Intercom_Controller::Intercom_Controller(Message_Handler& message_handler, PlfRegistry &registry) : 
	_message_handler(message_handler),
	_fsm_state(INTERCOM_CONTROLLER_FSM_STATE_RESTARTED), _prev_millis(0),
 	_registry(registry) {
	_message_handler.register_handler(I_AM_REPLY_T_MSG_ID, message_handler_helper, this, true);
	_message_handler.register_handler(ECHO_REQUEST_T_MSG_ID, message_handler_helper, this, true);
}
