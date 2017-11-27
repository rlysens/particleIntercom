#ifndef INTERCOM_BUDDY_H
#define INTERCOM_BUDDY_H

#include "Particle.h"
#include "message_handler.h"
#include "plf_registry.h"
#include "intercom_buttons_and_leds.h"
#include "intercom_outgoing.h"

#define NUM_BUDDIES 3

class IntercomBuddy {
private:
	Intercom_Outgoing *_intercom_outgoingp;
	Message_Handler* _message_handlerp;
	PlfRegistry* _registryp;
	IntercomButtonsAndLeds* _intercomButtonsAndLedsp;
	IntercomLed* _buddyLedp;

	int _buddyIdx;
	uint32_t _buddyId;
	int32_t _fsm_state;
	int32_t _echo_reply_acc;
	unsigned long _prev_millis;
	bool _initialized;

	void _txSetBuddy(void);
	void _txEchoReq(void);
	int _rxEchoRep(Intercom_Message& msg, int payload_size);
	void _txWhoIsReq(void);
	int _rxWhoIsRep(Intercom_Message& msg, int payload_size);
	void _fsm(void);
	

public:
	IntercomBuddy();

	void init(Intercom_Outgoing *intercom_outgoingp, Message_Handler* message_handlerp, 
		PlfRegistry *registryp, IntercomButtonsAndLeds* intercomButtonsAndLedsp, int buddyIndex);

	void tick(void);

	bool checkButtonAndSend(void);

	/*private*/
	int handle_message(Intercom_Message &msg, int payload_size);
};

#endif /*INTERCOM_BUDDY_H*/