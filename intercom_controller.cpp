#include "intercom_controller.h"
#include "messages.h"

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

	if (!_my_name_is_set)
		return;

	_my_name.getBytes((unsigned char*)i_am.name, sizeof(i_am.name));

	num_encoded_bytes = i_am_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &i_am);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	intercom_message.id = MSG_ID_I_AM;
	_message_handler.send(intercom_message, num_encoded_bytes);
}

void Intercom_Controller::_whois(void) {
	int num_encoded_bytes;
	static Intercom_Message intercom_message;
	static who_is_t who_is;

	if (!_buddy_name_is_set)
		return;

	_buddy_name.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	num_encoded_bytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	intercom_message.id = MSG_ID_WHO_IS;
	_message_handler.send(intercom_message, num_encoded_bytes);
}

int Intercom_Controller::_whois_reply(Intercom_Message& msg, int payload_size) {
	who_is_reply_t who_is_reply;

	int num_decoded_bytes = who_is_reply_t_decode(msg.data, 0, payload_size, &who_is_reply);

	if (num_decoded_bytes < 0)
		return -1;

	if (!_buddy_name_is_set)
		return -2;

	/*zero terminate*/
	who_is_reply.name[sizeof(who_is_reply.name)-1] = 0;

	if (!String((const char*)who_is_reply.name).equals(_buddy_name))
		return -3;

	_intercom_outgoing.set_target(who_is_reply.id);
	return 0;
}

int Intercom_Controller::handle_message(Intercom_Message& msg, int payload_size) {
	switch (msg.id) {
    case MSG_ID_WHO_IS_REPLY:
    	return _whois_reply(msg, payload_size);

    default:
      return -1;
  }

  return 0;
}

void Intercom_Controller::_onTimeout(void) {
	_i_am();
	_whois();
}

Intercom_Controller::Intercom_Controller(Message_Handler& message_handler, Intercom_Outgoing& intercom_outgoing) : 
	_message_handler(message_handler), _intercom_outgoing(intercom_outgoing), _my_name_is_set(false), _buddy_name_is_set(false),
	_timer(2000, &Intercom_Controller::_onTimeout, *this) {
	_message_handler.register_handler(MSG_ID_WHO_IS_REPLY, message_handler_helper, this);
	_timer.start();
}
