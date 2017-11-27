#ifndef INTERCOM_OUTGOING_H
#define INTERCOM_OUTGOING_H

#include "Particle.h"
#include "message_handler.h"

class Intercom_Outgoing {
private:
	Message_Handler& _message_handler;

public:
	Intercom_Outgoing(Message_Handler& message_handler);

	void transfer(uint32_t buddyId);
};

#endif /*INTERCOM_OUTGOING_H*/