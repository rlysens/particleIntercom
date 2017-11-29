#ifndef INTERCOM_CONTROLLER_H
#define INTERCOM_CONTROLLER_H

#include "intercom_message_handler.h"
#include "Particle.h"
#include "plf_registry.h"
#include "plf_ticker_base.h"

class Intercom_Controller : public Plf_TickerBase {
private:
	Intercom_MessageHandler& _messageHandler;
	int32_t _fsmState;
	uint32_t _myId;
	unsigned long _prevMillis;
	PlfRegistry& _registry;

	void _i_am(void);
	int _i_am_reply(Intercom_Message& msg, int payloadSize);
	int _rx_echo_request(Intercom_Message& msg, int payloadSize);

	virtual void _tickerHook(void);

public:
	Intercom_Controller(Intercom_MessageHandler& messageHandler, PlfRegistry &registry);
	
	/*private*/
	int handleMessage(Intercom_Message &msg, int payloadSize);
	int registryHandler(int key, String& value, bool valid);
};

#endif /*INTERCOM_CONTROLLER_H*/