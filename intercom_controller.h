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
	Timer _timer;
	
	void _i_am(void);
	void _whois(void);
	int _whois_reply(Intercom_Message &msg, int payload_size);
	void _onTimeout(void);

public:
	Intercom_Controller(Message_Handler& message_handler, Intercom_Outgoing& intercom_outgoing);

	int handle_message(Intercom_Message &msg, int payload_size);

	void set_my_name(String& name);
	void set_buddy_name(String& name);
};

#endif /*INTERCOM_CONTROLLER_H*/