#ifndef INTERCOM_INCOMING_H
#define INTERCOM_INCOMING_H

#include "message_handler.h"

class Intercom_Incoming {
private:
  Message_Handler& _message_handler;

  int _receive(int8_t *rx_data, int rx_data_length);

public:
  Intercom_Incoming(Message_Handler& message_handler);

  void drain(void);

  /*private*/
  int handle_message(Intercom_Message &msg, int payload_size);
};

#endif /*INTERCOM_INCOMING_H*/
