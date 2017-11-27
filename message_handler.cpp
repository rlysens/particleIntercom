#include "message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"

Intercom_Message intercom_message; /*Shared by all message_handler users*/

static int registryHandlerHelper(int key, String& value, bool valid, void *ctxt) {
  Message_Handler *msg_handlerp = (Message_Handler*)ctxt;

  PLF_PRINT(PRNTGRP_DFLT,"In registryHandlerHelper\n");

  plf_assert("msg_handler NULL ptr", msg_handlerp);
  plf_assert("invalid reg key", key == REG_KEY_SECRET_KEY);

  if (valid) {
    uint8_t keyArray[17]; /*+1 because getBytes add zero terminator*/
    value.getBytes(keyArray, sizeof(keyArray));
    msg_handlerp->set_encryption_key(keyArray);
  }

  return 0;
}


void Message_Handler::set_encryption_key(uint8_t key[16]) {
  _encryption_key_set = true;
  mbedtls_xtea_setup( &_xtea_ctxt, key );
}

int Message_Handler::_encrypt_msg(Intercom_Message &msg, int payload_size) {
  int result=-1;

  if (_encryption_key_set) {
    result = mbedtls_xtea_crypt_cbc(&_xtea_ctxt,
      MBEDTLS_XTEA_ENCRYPT,
      payload_size,
      _iv_enc,
      msg.data,
      msg.data);
  }

  return result;
}

int Message_Handler::_decrypt_msg(Intercom_Message &msg, int payload_size) {
  int result=-1;

  if (_encryption_key_set) {
    result = mbedtls_xtea_crypt_cbc(&_xtea_ctxt,
      MBEDTLS_XTEA_DECRYPT,
      payload_size,
      _iv_dec,
      msg.data,
      msg.data);

    if (result!=0) {
      PLF_PRINT(PRNTGRP_MSGS, "Decrypt failed, res=%d.\n", result);
    }
  }
  else {
    PLF_PRINT(PRNTGRP_MSGS, "Encryption key not set.\n");
  }

  return result;
}

void Message_Handler::setMyId(uint32_t my_id) {
  _my_id = my_id;
}

uint32_t Message_Handler::getMyId(void) {
  return _my_id;
}

int Message_Handler::send(Intercom_Message &msg, uint32_t msg_id, int payload_size, bool encrypted) {

  PLF_PRINT(PRNTGRP_MSGS, "Tx Msg %d\n", (int)msg_id);

  msg.msg_id = msg_id;
  msg.source_id = _my_id;

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
    if (_encrypt_msg(msg, payload_size)!=0) {
      return -3;
    }
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
  MessageHandlerTableElement *msgEntryp = &_msgTable[msg.msg_id];
  int ii;

  if (rx_data_length < 8)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rx_data_length);
  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %d\n", (int)msg.msg_id);

  payload_size = rx_data_length - 8;

  if (msgEntryp->top_index==0)
    return -100; /*No handlers for this message*/

  if (msgEntryp->encrypted) {
    if (_decrypt_msg(msg, payload_size) != 0) {
      return -101;
    }
  }

  for (ii=0; ii<msgEntryp->top_index; ++ii) {
    if (msgEntryp->fun[ii]) {
  	  msgEntryp->fun[ii](msg, payload_size, msgEntryp->ctxt[ii]);
    }
  }

  return 0;
}

int Message_Handler::register_handler(uint16_t id, 
	MessageHandlerFunType *fun, void *ctxt, bool encrypted) {

	plf_assert("Msg ID too large", id <= MAX_MESSAGE_ID);

  MessageHandlerTableElement *msgEntryp = &_msgTable[id];

  plf_assert("Too many handlers", msgEntryp->top_index<MAX_NUM_FUNS_PER_MSG);

	msgEntryp->fun[msgEntryp->top_index] = fun;
	msgEntryp->ctxt[msgEntryp->top_index] = ctxt;
  msgEntryp->encrypted = encrypted;
  ++(msgEntryp->top_index);

  return 0;
}

Message_Handler::Message_Handler(int local_port, 
	IPAddress remote_ip_address, int remote_port, PlfRegistry& registry) : 
	_remote_ip_address(remote_ip_address),
	_remote_port(remote_port), _msgTable(), _my_id(ID_UNKNOWN), _registry(registry),
  _encryption_key_set(false) {

	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer\n");
  }

  memset(_iv_enc, 0, sizeof(_iv_enc));
  memset(_iv_dec, 0, sizeof(_iv_dec));

  mbedtls_xtea_init(&_xtea_ctxt);

  _registry.registerHandler(REG_KEY_SECRET_KEY, registryHandlerHelper, this);

	_udp.begin(local_port);
}