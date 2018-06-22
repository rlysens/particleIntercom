#ifndef INTERCOM_OUTGOING_H
#define INTERCOM_OUTGOING_H

#include "Particle.h"
#include "intercom_message_handler.h"
#include "intercom_buddy.h"
#include "messages.h"

#define INTERCOM_OUTGOING_NUM_REQ_IDS 2
#define NUM_RETRANSMIT_BUFFERS 15

typedef struct {
	Intercom_Message msg;
	uint32_t numEncodedBytes;
	int32_t seqNumber;
	int32_t destinationId;
	IPAddress buddyServerAddr;

} RetransmitEntry_t;

class Intercom_Outgoing {
private:
	Intercom_MessageHandler* _messageHandlerp;
	Intercom_Buddy* _intercom_buddiesp;
	RetransmitEntry_t _retransmitBuffers[NUM_RETRANSMIT_BUFFERS];

	int _fsmState;
	int _numBytesSentAcc;
	uint32_t _seqNumber;
	uint32_t _retransmitBufferIndex;
	bool _initialized;
	
	void _fsmUpdate(void);
	void _dataDump(void);
	int _rxRetransmitReq(Intercom_Message& msg, int payloadSize);
	int _handleMessage(Intercom_Message &msg, int payloadSize);

public:
	Intercom_Outgoing();

	void init(Intercom_MessageHandler& messageHandler, Intercom_Buddy intercom_buddies[NUM_BUDDIES]);

	void run(void);
};

#endif /*INTERCOM_OUTGOING_H*/