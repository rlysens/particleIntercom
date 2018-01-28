#ifndef INTERCOM_BUDDY_H
#define INTERCOM_BUDDY_H

#include "Particle.h"
#include "intercom_message_handler.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_outgoing.h"
#include "intercom_incoming.h"
#include "board.h"
#include "plf_ticker_base.h"

class Intercom_Buddy : public Plf_TickerBase {
private:
	Intercom_Outgoing *_intercom_outgoingp;
	Intercom_Incoming *_intercom_incomingp;
	Intercom_MessageHandler* _messageHandlerp;
	Intercom_ButtonsAndLeds* _intercom_buttonsAndLedsp;
	Intercom_Led* _buddyLedp;

	int _buddyIdx;
	uint32_t _buddyId;
	int32_t _listeningState;
	int32_t _commState;
	int32_t _ledState;
	int32_t _echoReplyAcc;
	unsigned long _prevMillis;
	int _buttonState;
	int _tickCount;
	bool _sendCommStart;
	bool _sendCommStop;
	bool _initialized;

	int _rxCommStart(Intercom_Message& msg, int payloadSize);
	int _rxCommStop(Intercom_Message& msg, int payloadSize);
	void _txCommStop(void);
	void _txCommStart(void);
	int _rxCommStartAck(Intercom_Message& msg, int payloadSize);
	int _rxCommStopAck(Intercom_Message& msg, int payloadSize);
	void _txSetBuddy(void);
	void _txEchoReq(void);
	int _rxEchoRep(Intercom_Message& msg, int payloadSize);
	void _txWhoIsReq(void);
	int _rxWhoIsRep(Intercom_Message& msg, int payloadSize);
	void _listeningStateUpdate(void);
	void _buddyLedUpdate(void);
	void _commStateSuspendCheck(void);

	virtual void _tickerHook(void);

	void _dataDump(void);

public:
	Intercom_Buddy();

	void init(Intercom_Outgoing *intercom_outgoingp, 
		Intercom_Incoming *intercom_incomingp, Intercom_MessageHandler* messageHandlerp, 
		Intercom_ButtonsAndLeds* intercom_buttonsAndLedsp, int buddyIndex);

	bool checkButtonAndSend(void);

	/*private*/
	int handleMessage(Intercom_Message &msg, int payloadSize);
};

#endif /*INTERCOM_BUDDY_H*/