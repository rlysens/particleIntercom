#ifndef INTERCOM_OUTGOING_H
#define INTERCOM_OUTGOING_H

#include "Particle.h"
#include "intercom_message_handler.h"

#define INTERCOM_OUTGOING_NUM_REQ_IDS 2

class Intercom_Outgoing {
private:
	Intercom_MessageHandler& _messageHandler;
	int _fsmState;
	int _numBytesSentAcc;
	uint32_t _seqNumber;
	bool _recordRequests[INTERCOM_OUTGOING_NUM_REQ_IDS];

	void _fsmUpdate(void);
	void _dataDump(void);
	
public:
	Intercom_Outgoing(Intercom_MessageHandler& messageHandler);

	void run(uint32_t buddyId);

	void recordRequest(unsigned requesterId, bool enable);
};

#endif /*INTERCOM_OUTGOING_H*/