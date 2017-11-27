#ifndef INTERCOM_CONTROLLER_H
#define INTERCOM_CONTROLLER_H

#include "message_handler.h"
#include "Particle.h"
#include "plf_registry.h"

class Intercom_Controller {
private:
	Message_Handler& _message_handler;
	int32_t _fsm_state;
	uint32_t _my_id;
	unsigned long _prev_millis;
	PlfRegistry& _registry;

	void _i_am(void);
	int _i_am_reply(Intercom_Message& msg, int payload_size);
	int _rx_echo_request(Intercom_Message& msg, int payload_size);

public:
	Intercom_Controller(Message_Handler& message_handler, PlfRegistry &registry);

	void tick(void);
	
	/*private*/
	int handle_message(Intercom_Message &msg, int payload_size);
	int registry_handler(int key, String& value, bool valid);
};

#endif /*INTERCOM_CONTROLLER_H*/