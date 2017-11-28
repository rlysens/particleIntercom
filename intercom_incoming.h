#ifndef INTERCOM_INCOMING_H
#define INTERCOM_INCOMING_H

#include "intercom_message_handler.h"
#include "plf_circular_buffer.h"

#define CIRCULAR_BUFFER_SIZE (8192*2)

class Intercom_Incoming {
private:
	uint8_t _circularBuffer[CIRCULAR_BUFFER_SIZE];
	Plf_CircularBuf _circularBuf;

  Intercom_MessageHandler& _messageHandler;

  int _receive(int8_t *rxData, int rxDataLength);

public:
  Intercom_Incoming(Intercom_MessageHandler& messageHandler);

  void drain(void);

  /*private*/
  int handleMessage(Intercom_Message &msg, int payloadSize);
};

#endif /*INTERCOM_INCOMING_H*/
