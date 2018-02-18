#ifndef INTERCOM_OUTGOING_H
#define INTERCOM_OUTGOING_H

#include "Particle.h"
#include "intercom_message_handler.h"
#include "intercom_buddy.h"

#define INTERCOM_OUTGOING_NUM_REQ_IDS 2

class Intercom_Outgoing {
private:
	Intercom_MessageHandler* _messageHandlerp;
	Intercom_Buddy* _intercom_buddiesp;

	int _fsmState;
	int _numBytesSentAcc;
	uint32_t _seqNumber;
	bool _initialized;
	
	void _fsmUpdate(void);
	void _dataDump(void);
	
public:
	Intercom_Outgoing();

	void init(Intercom_MessageHandler& messageHandler, Intercom_Buddy intercom_buddies[NUM_BUDDIES]);

	void run(void);
};

#endif /*INTERCOM_OUTGOING_H*/