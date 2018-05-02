#ifndef INTERCOM_CONTROLLER_H
#define INTERCOM_CONTROLLER_H

#include "intercom_message_handler.h"
#include "Particle.h"
#include "plf_ticker_base.h"

class Intercom_Controller : public Plf_TickerBase {
private:
	Intercom_MessageHandler& _messageHandler;
	IPAddress _myServerAddress;

	void _i_am(void);

	virtual void _tickerHook(void);

	void _dataDump(void);
	int _registryHandlerSrvrName(int key);
	
public:
	Intercom_Controller(Intercom_MessageHandler& messageHandler);
	
	/*private*/
	int handleMessage(Intercom_Message &msg, int payloadSize);
};

#endif /*INTERCOM_CONTROLLER_H*/