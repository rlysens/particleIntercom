#ifndef INTERCOM_CONTROLLER_H
#define INTERCOM_CONTROLLER_H

#include "message_handler.h"
#include "intercom_outgoing.h"
#include "Particle.h"
#include "plf_registry.h"

class Intercom_Controller {
private:
	Message_Handler& _message_handler;
	Intercom_Outgoing& _intercom_outgoing;
	int32_t _my_id;
	bool _my_id_is_known;
	unsigned long _prev_millis;
	PlfRegistry& _registry;

	void _i_am(void);
	void _whois(void);
	int _whois_reply(Intercom_Message &msg, int payload_size);
	int _i_am_reply(Intercom_Message& msg, int payload_size);
	void _buddy_listening_led(bool on);
	void _tx_echo_req(void);
	int _echo_reply(Intercom_Message& msg, int payload_size);
	int _rx_echo_request(Intercom_Message& msg, int payload_size);

public:
	Intercom_Controller(Message_Handler& message_handler, 
		Intercom_Outgoing& intercom_outgoing, PlfRegistry &registry);

	void tick(void);
	
	/*private*/
	int handle_message(Intercom_Message &msg, int payload_size);
	int registry_handler(int key, String& value, bool valid);
};

#endif /*INTERCOM_CONTROLLER_H*/