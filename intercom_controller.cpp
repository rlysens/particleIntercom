#include "intercom_controller.h"
#include "messages.h"

static Intercom_Message intercom_message;

static int message_handler_helper(Intercom_Message &msg, 
  int payload_size, void *ctxt) {
	Intercom_Controller *intercom_controllerp = (Intercom_Controller*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_controllerp);

  return intercom_controllerp->handle_message(msg, payload_size);
}

void Intercom_Controller::set_my_name(String& name) {
	_my_name = name;
	_my_name_is_set = true;
}

void Intercom_Controller::set_buddy_name(String& name) {
	_buddy_name = name;
	_buddy_name_is_set = true;
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

	intercom_message.id = I_AM_T_MSG_ID;
	_message_handler.send(intercom_message, num_encoded_bytes);
}

void Intercom_Controller::_whois(void) {
	int num_encoded_bytes;
	static who_is_t who_is;

	if (!_buddy_name_is_set)
		return;

	_buddy_name.getBytes((unsigned char*)who_is.name, sizeof(who_is.name));

	num_encoded_bytes = who_is_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &who_is);
	plf_assert("Msg Encode Error", num_encoded_bytes>=0);

	intercom_message.id = WHO_IS_T_MSG_ID;
	_message_handler.send(intercom_message, num_encoded_bytes);
}

int Intercom_Controller::_whois_reply(Intercom_Message& msg, int payload_size) {
	who_is_reply_t who_is_reply;
	add_buddy_t add_buddy;
	int num_encoded_bytes;
	int num_decoded_bytes = who_is_reply_t_decode(msg.data, 0, payload_size, &who_is_reply);

	if (num_decoded_bytes < 0)
		return -1;

	if (!_buddy_name_is_set)
		return -2;

	/*zero terminate*/
	who_is_reply.name[sizeof(who_is_reply.name)-1] = 0;

	if (!String((const char*)who_is_reply.name).equals(_buddy_name))
		return -3;

	if (!_buddy_id_is_known) {
		PLF_PRINT("Buddy id received %d\n", (int)who_is_reply.id);
	}

	if (_buddy_id_is_known) {
		/*Did buddy id change?*/
		if (_buddy_id != who_is_reply.id) {
			if (_my_id_is_known) {
				del_buddy_t del_buddy;

				/*Remove old buddy server side*/
				del_buddy.my_id = _my_id;
				del_buddy.buddy_id = who_is_reply.id;

				num_encoded_bytes = del_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &del_buddy);
				plf_assert("Msg Encode Error", num_encoded_bytes>=0);

				intercom_message.id = DEL_BUDDY_T_MSG_ID;
				_message_handler.send(intercom_message, num_encoded_bytes);
			}
		}
	}

	_buddy_id_is_known = true;
	_buddy_id = who_is_reply.id;

	if (_my_id_is_known) {
		/*Add my buddy to list server side*/
		add_buddy.my_id = _my_id;
		add_buddy.buddy_id = who_is_reply.id;

		num_encoded_bytes = add_buddy_t_encode(intercom_message.data, 0, sizeof(intercom_message.data), &add_buddy);
		plf_assert("Msg Encode Error", num_encoded_bytes>=0);

		intercom_message.id = ADD_BUDDY_T_MSG_ID;
		_message_handler.send(intercom_message, num_encoded_bytes);
	}

	_intercom_outgoing.set_destination_id(who_is_reply.id);
	return 0;
}

int Intercom_Controller::_i_am_reply(Intercom_Message& msg, int payload_size) {
	i_am_reply_t i_am_reply;

	int num_decoded_bytes = i_am_reply_t_decode(msg.data, 0, payload_size, &i_am_reply);

	if (num_decoded_bytes < 0)
		return -1;

	if (!String((const char*)i_am_reply.name).equals(_my_name))
		return -2;

	if (!_my_id_is_known) {
		PLF_PRINT("My id received %d\n", (int)i_am_reply.id);
	}

	_my_id_is_known = true;
	_my_id = i_am_reply.id;

	_intercom_outgoing.set_source_id(i_am_reply.id);

	return 0;
}

int Intercom_Controller::handle_message(Intercom_Message& msg, int payload_size) {
	switch (msg.id) {
    case WHO_IS_REPLY_T_MSG_ID:
    	return _whois_reply(msg, payload_size);

    case I_AM_REPLY_T_MSG_ID:
    	return _i_am_reply(msg, payload_size);
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
	_timer(2000, &Intercom_Controller::_onTimeout, *this), _my_id_is_known(false), _buddy_id_is_known(false) {
	_message_handler.register_handler(WHO_IS_REPLY_T_MSG_ID, message_handler_helper, this);
	_message_handler.register_handler(I_AM_REPLY_T_MSG_ID, message_handler_helper, this);
	_timer.start();
}
