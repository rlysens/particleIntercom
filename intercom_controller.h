#ifndef INTERCOM_CONTROLLER_H
#define INTERCOM_CONTROLLER_H

#include "message_handler.h"
#include "intercom_outgoing.h"
#include "Particle.h"

class Intercom_Controller {
private:
	Message_Handler& _message_handler;
	Intercom_Outgoing& _intercom_outgoing;
	bool _my_name_is_set;
	String _my_name;
	bool _buddy_name_is_set;
	String _buddy_name;
	int32_t _my_id;
	bool _my_id_is_known;
	int32_t _buddy_id;
	bool _buddy_id_is_known;
	unsigned long _prev_millis;
	
	void _i_am(void);
	void _whois(void);
	int _whois_reply(Intercom_Message &msg, int payload_size);
	int _i_am_reply(Intercom_Message& msg, int payload_size);

public:
	Intercom_Controller(Message_Handler& message_handler, Intercom_Outgoing& intercom_outgoing);

	void tick(void);
	int handle_message(Intercom_Message &msg, int payload_size);

	void set_my_name(String& name);
	void set_buddy_name(String& name);
};

#endif /*INTERCOM_CONTROLLER_H*/