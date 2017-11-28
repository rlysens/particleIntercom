#ifndef INTERCOM_OUTGOING_H
#define INTERCOM_OUTGOING_H

#include "Particle.h"
#include "intercom_message_handler.h"

class Intercom_Outgoing {
private:
	Intercom_MessageHandler& _messageHandler;

public:
	Intercom_Outgoing(Intercom_MessageHandler& messageHandler);

	void transfer(uint32_t buddyId);
};

#endif /*INTERCOM_OUTGOING_H*/