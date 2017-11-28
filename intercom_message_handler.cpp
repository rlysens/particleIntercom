#include "intercom_message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"

Intercom_Message intercom_message; /*Shared by all message_handler users*/

static int registryHandlerHelper(int key, String& value, bool valid, void *ctxt) {
  Intercom_MessageHandler *msgHandlerp = (Intercom_MessageHandler*)ctxt;

  PLF_PRINT(PRNTGRP_DFLT,"In registryHandlerHelper\n");

  plf_assert("msgHandlerp NULL ptr", msgHandlerp);
  plf_assert("invalid reg key", key == REG_KEY_SECRET_KEY);

  if (valid) {
    uint8_t keyArray[17]; /*+1 because getBytes add zero terminator*/
    value.getBytes(keyArray, sizeof(keyArray));
    msgHandlerp->setEncryptionKey(keyArray);
  }

  return 0;
}


void Intercom_MessageHandler::setEncryptionKey(uint8_t key[16]) {
  _encryptionKeyIsSet = true;
  mbedtls_xtea_setup( &_xteaCtxt, key );
}

int Intercom_MessageHandler::_encryptMsg(Intercom_Message &msg, int payloadSize) {
  int result=-1;

  if (_encryptionKeyIsSet) {
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_ENCRYPT,
      payloadSize,
      _ivEnc,
      msg.data,
      msg.data);
  }

  return result;
}

int Intercom_MessageHandler::_decryptMsg(Intercom_Message &msg, int payloadSize) {
  int result=-1;

  if (_encryptionKeyIsSet) {
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_DECRYPT,
      payloadSize,
      _ivDec,
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

void Intercom_MessageHandler::setMyId(uint32_t myId) {
  _myId = myId;
}

uint32_t Intercom_MessageHandler::getMyId(void) {
  return _myId;
}

int Intercom_MessageHandler::send(Intercom_Message &msg, uint32_t msgId, int payloadSize, bool encrypted) {

  PLF_PRINT(PRNTGRP_MSGS, "Tx Msg %d\n", (int)msgId);

  msg.msg_id = msgId;
  msg.source_id = _myId;

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
    if (_encryptMsg(msg, payloadSize)!=0) {
      return -3;
    }
  }

  /* Send the UDP packet */
  if (_udp.sendPacket((uint8_t*)&msg, payloadSize+8, 
  		_remoteIpAddress, _remotePort) != payloadSize+8) {
      PLF_PRINT(PRNTGRP_DFLT, ("UDP packet send failed. Could not send all data\n"));
      return -2;
  }

  PLF_COUNT_VAL(UDP_BYTES_TX, payloadSize);
  
  return 0;
}

int Intercom_MessageHandler::receive(void) {
	static Intercom_Message msg;
  int rxDataLength = _udp.receivePacket((uint8_t*)&msg, sizeof(msg));
  int payloadSize;
  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[msg.msg_id];
  int ii;

  if (rxDataLength < 8)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rxDataLength);
  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %d\n", (int)msg.msg_id);

  payloadSize = rxDataLength - 8;

  if (msgEntryp->topIndex==0)
    return -100; /*No handlers for this message*/

  if (msgEntryp->encrypted) {
    if (_decryptMsg(msg, payloadSize) != 0) {
      return -101;
    }
  }

  for (ii=0; ii<msgEntryp->topIndex; ++ii) {
    if (msgEntryp->fun[ii]) {
  	  msgEntryp->fun[ii](msg, payloadSize, msgEntryp->ctxt[ii]);
    }
  }

  return 0;
}

int Intercom_MessageHandler::registerHandler(uint16_t id, 
	Intercom_MessageHandlerFunType *fun, void *ctxt, bool encrypted) {

	plf_assert("Msg ID too large", id <= MAX_MESSAGE_ID);

  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[id];

  plf_assert("Too many handlers", msgEntryp->topIndex<MAX_NUM_FUNS_PER_MSG);

	msgEntryp->fun[msgEntryp->topIndex] = fun;
	msgEntryp->ctxt[msgEntryp->topIndex] = ctxt;
  msgEntryp->encrypted = encrypted;
  ++(msgEntryp->topIndex);

  return 0;
}

Intercom_MessageHandler::Intercom_MessageHandler(int localPort, 
	IPAddress remoteIpAddress, int remotePort, PlfRegistry& registry) : 
	_remoteIpAddress(remoteIpAddress),
	_remotePort(remotePort), _msgTable(), _myId(ID_UNKNOWN), _registry(registry),
  _encryptionKeyIsSet(false) {

	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer\n");
  }

  memset(_ivEnc, 0, sizeof(_ivEnc));
  memset(_ivDec, 0, sizeof(_ivDec));

  mbedtls_xtea_init(&_xteaCtxt);

  _registry.registerHandler(REG_KEY_SECRET_KEY, registryHandlerHelper, this);

	_udp.begin(localPort);
}