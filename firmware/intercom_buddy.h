#ifndef INTERCOM_BUDDY_H
#define INTERCOM_BUDDY_H

#include "Particle.h"
#include "intercom_message_handler.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_incoming.h"
#include "board.h"
#include "plf_ticker_base.h"

#define INTERCOM_BUDDY_OUTGOING_REQ_TYPE_BUTTON 0
#define INTERCOM_BUDDY_OUTGOING_REQ_TYPE_INCOMING_COMM 1
#define INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES 2

class Intercom_Buddy : public Plf_TickerBase {
private:
	Intercom_Incoming *_intercom_incomingp;
	Intercom_MessageHandler* _messageHandlerp;
	Intercom_ButtonsAndLeds* _intercom_buttonsAndLedsp;
	Intercom_Led* _buddyLedp;
	Intercom_TxMessageCounter* _txMsgCounterp;
	Intercom_RxMessageCounter* _rxMsgCounterp;
	uint32_t _prevTxMsgCounter;
	int _buddyIdx;
	uint32_t _buddyId;
	int32_t _listeningState;
	int32_t _incomingCommState;
	int32_t _ledState;
	int32_t _outgoingCommFsmState;
	unsigned long _prevMillis;
	IPAddress _buddyServerAddress;
	IPAddress _myServerAddress;
	int _buttonState;
	int _tickCount;
	bool _sendCommStart;
	bool _sendCommStop;
	bool _outgoingCommRequests[INTERCOM_BUDDY_NUM_OUTGOING_REQ_TYPES];
	bool _sendSetBuddy;
	bool _initialized;

	int _rxKeepAliveResp(Intercom_Message& msg, int payloadSize);
	int _rxCommStart(Intercom_Message& msg, int payloadSize);
	int _rxCommStop(Intercom_Message& msg, int payloadSize);
	void _txCommStop(void);
	void _txCommStart(void);
	int _rxCommStartAck(Intercom_Message& msg, int payloadSize);
	int _rxCommStopAck(Intercom_Message& msg, int payloadSize);
	int _rxSetBuddyAck(Intercom_Message& msg, int payloadSize);
	int _setBuddy(int key);
	void _txSetBuddy(void);
	void _txKeepAlive(void);
	void _listeningStateUpdate(void);
	void _buddyLedUpdate(void);
	void _incomingCommStateSuspendCheck(void);
	void _outgoingCommRequest(unsigned requestType, bool enable);

	virtual void _tickerHook(void);

	int _setServerAddr(int key); 

	void _dataDump(void);
	int _handleMessage(Intercom_Message &msg, int payloadSize);

public:
	Intercom_Buddy();

	void init(Intercom_Incoming *intercom_incomingp, Intercom_MessageHandler* messageHandlerp, 
		Intercom_ButtonsAndLeds* intercom_buttonsAndLedsp, int buddyIndex);

	bool checkButton(void);

	/*Returns true if outgoing communication is requested for this buddy*/
	bool outgoingCommRequested(void);

	inline uint32_t getBuddyId(void) {
		return _buddyId;
	}

	IPAddress& getBuddyServerAddress(void) {
		return _buddyServerAddress;
	}
};

#endif /*INTERCOM_BUDDY_H*/