#ifndef INTERCOM_PROXY_H
#define INTERCOM_PROXY_H

#include "Particle.h"
#include "plf_utils.h"
#include "message_handler.h"

class Intercom_Outgoing {
private:
	Message_Handler _message_handler;

public:
  	Intercom_Outgoing(Message_Handler& message_handler);
  	void transfer(void);
};

#endif /*INTERCOM_PROXY_H*/
