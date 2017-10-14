#include "message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"

void Message_Handler::set_encryption_key(uint8_t key[16]) {
  mbedtls_xtea_setup( &_xtea_ctxt, key );
}

void Message_Handler::_encrypt_msg(Intercom_Message &msg, int payload_size) {
  int result = mbedtls_xtea_crypt_cbc(&_xtea_ctxt,
                    MBEDTLS_XTEA_ENCRYPT,
                    payload_size,
                    _iv_enc,
                    msg.data,
                    msg.data);

  plf_assert("Encryption failed", result == 0);
}

void Message_Handler::_decrypt_msg(Intercom_Message &msg, int payload_size) {
  int result = mbedtls_xtea_crypt_cbc(&_xtea_ctxt,
                    MBEDTLS_XTEA_DECRYPT,
                    payload_size,
                    _iv_dec,
                    msg.data,
                    msg.data);

  plf_assert("Decryption failed", result == 0);
}

void Message_Handler::set_source_id(uint32_t source_id) {
  _source_id = source_id;
}

int Message_Handler::send(Intercom_Message &msg, uint32_t msg_id, int payload_size, bool encrypted) {

  PLF_PRINT(PRNTGRP_MSGS, "Tx Msg %d\n", (int)msg_id);

  msg.msg_id = msg_id;
  msg.source_id = _source_id;

#if 0
  {
    uint8_t *p = (uint8_t*)&msg;
    int i;
    for (i=0;i<16;i++) {
      PLF_PRINT(PRNTGRP_DFLT,"%x", p[i]);
    }
    PLF_PRINT(PRNTGRP_DFLT,"\n");
  }
#endif
  
  if (encrypted) {
    _encrypt_msg(msg, payload_size);
  }

  /* Send the UDP packet */
  if (_udp.sendPacket((uint8_t*)&msg, payload_size+8, 
  		_remote_ip_address, _remote_port) != payload_size+8) {
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

  if (rx_data_length < 8)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rx_data_length);

  payload_size = rx_data_length - 8;

  if (_msgTable[msg.msg_id].fun == 0) {
  	return -1;
  }

  if (_msgTable[msg.msg_id].encrypted) {
    _decrypt_msg(msg, payload_size);
  }

  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %d\n", (int)msg.msg_id);

  /*Dispatch*/
  return _msgTable[msg.msg_id].fun(msg, payload_size, _msgTable[msg.msg_id].ctxt);
}

int Message_Handler::register_handler(uint16_t id, 
	MessageHandlerFunType *fun, void *ctxt, bool encrypted) {

	plf_assert("Msg ID too large", id <= MAX_MESSAGE_ID);

	_msgTable[id].fun = fun;
	_msgTable[id].ctxt = ctxt;
  _msgTable[id].encrypted = encrypted;

  return 0;
}

Message_Handler::Message_Handler(int local_port, 
	IPAddress remote_ip_address, int remote_port) : 
	_remote_ip_address(remote_ip_address),
	_remote_port(remote_port), _msgTable(), _source_id(~0UL) {

	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer\n");
  }

  memset(_iv_enc, 0, sizeof(_iv_enc));
  memset(_iv_dec, 0, sizeof(_iv_dec));

  mbedtls_xtea_init(&_xtea_ctxt);

	_udp.begin(local_port);
}