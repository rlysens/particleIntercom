#ifndef INTERCOM_BUDDY_H
#define INTERCOM_BUDDY_H

#include "Particle.h"
#include "intercom_message_handler.h"
#include "plf_registry.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_outgoing.h"
#include "board.h"
#include "plf_ticker_base.h"

class Intercom_Buddy : public Plf_TickerBase {
private:
	Intercom_Outgoing *_intercom_outgoingp;
	Intercom_MessageHandler* _messageHandlerp;
	PlfRegistry* _registryp;
	Intercom_ButtonsAndLeds* _intercom_buttonsAndLedsp;
	Intercom_Led* _buddyLedp;

	int _buddyIdx;
	uint32_t _buddyId;
	int32_t _fsmState;
	int32_t _echoReplyAcc;
	unsigned long _prevMillis;
	bool _initialized;

	void _txSetBuddy(void);
	void _txEchoReq(void);
	int _rxEchoRep(Intercom_Message& msg, int payloadSize);
	void _txWhoIsReq(void);
	int _rxWhoIsRep(Intercom_Message& msg, int payloadSize);
	void _fsm(void);
	
	virtual void _tickerHook(void);

public:
	Intercom_Buddy();

	void init(Intercom_Outgoing *intercom_outgoingp, Intercom_MessageHandler* messageHandlerp, 
		PlfRegistry *registryp, Intercom_ButtonsAndLeds* intercom_buttonsAndLedsp, int buddyIndex);

	bool checkButtonAndSend(void);

	/*private*/
	int handleMessage(Intercom_Message &msg, int payloadSize);
};

#endif /*INTERCOM_BUDDY_H*/