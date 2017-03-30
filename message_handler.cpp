#include "message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"

int Message_Handler::send(Intercom_Message &msg, int payload_size) {

  PLF_PRINT(PRNTGRP_MSGS, "Tx Msg %d\n", (int)msg.id);

  /* Send the UDP packet */
  if (_udp.sendPacket((uint8_t*)&msg, payload_size+4, 
  		_remote_ip_address, _remote_port) != payload_size+4) {
      PLF_PRINT(PRNTGRP_DFLT, ("UDP packet send failed. Could not send all data\n"));
      return -2;
  }

  PLF_COUNT_VAL(UDP_BYTES_TX, payload_size);
  
  return 0;
}

int Message_Handler::receive(void) {
	static Intercom_Message msg;
  int rx_data_length = _udp.receivePacket((uint8_t*)&msg, sizeof(msg));
  int payload_size;

  if (rx_data_length < 4)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rx_data_length);

  payload_size = rx_data_length - 4;

  if (_msgTable[msg.id].fun == 0) {
  	return -1;
  }

  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %d\n", (int)msg.id);

  /*Dispatch*/
  return _msgTable[msg.id].fun(msg, payload_size, _msgTable[msg.id].ctxt);
}

int Message_Handler::register_handler(uint16_t id, 
	MessageHandlerFunType *fun, void *ctxt) {

	plf_assert("Msg ID too large", id <= MAX_MESSAGE_ID);

	_msgTable[id].fun = fun;
	_msgTable[id].ctxt = ctxt;

  return 0;
}

Message_Handler::Message_Handler(int local_port, 
	IPAddress remote_ip_address, int remote_port) : 
	_remote_ip_address(remote_ip_address),
	_remote_port(remote_port), _msgTable() {

	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer\n");
    }

	_udp.begin(local_port);
}