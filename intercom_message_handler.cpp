#include "intercom_message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 600

Intercom_Message intercom_message; /*Shared by all message_handler users*/

int Intercom_MessageHandler::_registryHandler(int key, String& value, bool valid) {
  plf_assert("invalid reg key", key == REG_KEY_SECRET_KEY);

  if (valid) {
    uint8_t keyArray[17]; /*+1 because getBytes add zero terminator*/
    value.getBytes(keyArray, sizeof(keyArray));
    setEncryptionKey(keyArray);
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
#ifdef MBEDTLS_CIPHER_MODE_CBC
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_ENCRYPT,
      payloadSize,
      _ivEnc,
      msg.data,
      msg.data);
#else /*ECB mode:*/
    uint8_t *inputp = msg.data;

    plf_assert("payloadSize must be multiple of 8", (payloadSize%8)==0);

    result = 0;
    while (payloadSize > 0) {
      result |= mbedtls_xtea_crypt_ecb(&_xteaCtxt,
        MBEDTLS_XTEA_ENCRYPT,
        inputp,
        inputp);
      inputp += 8;
      payloadSize -= 8;
    }
#endif /*MBEDTLS_CIPHER_MODE_CBC*/
  }

  return result;
}

int Intercom_MessageHandler::_decryptMsg(Intercom_Message &msg, int payloadSize) {
  int result=-(MODULE_ID+1);

  if (_encryptionKeyIsSet) {
#ifdef MBEDTLS_CIPHER_MODE_CBC
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_DECRYPT,
      payloadSize,
      _ivDec,
      msg.data,
      msg.data);
#else /*ECB mode:*/
    uint8_t *inputp = msg.data;

    plf_assert("payloadSize must be multiple of 8", (payloadSize%8)==0);
    
    result = 0;
    while (payloadSize > 0) {
      result |= mbedtls_xtea_crypt_ecb(&_xteaCtxt,
        MBEDTLS_XTEA_DECRYPT,
        inputp,
        inputp);
      inputp += 8;
      payloadSize -= 8;
    }
#endif /*MBEDTLS_CIPHER_MODE_CBC*/
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

  msg.msgId = msgId;
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
      return -(MODULE_ID+2);
    }
  }

  /* Send the UDP packet */
  if (_udp.sendPacket((uint8_t*)&msg, payloadSize+8, 
  		_remoteIpAddress, _remotePort) != payloadSize+8) {
      PLF_PRINT(PRNTGRP_DFLT, ("UDP packet send failed. Could not send all data\n"));
      return -(MODULE_ID+3);
  }

  PLF_COUNT_VAL(UDP_BYTES_TX, payloadSize);
  
  return 0;
}

int Intercom_MessageHandler::receive(void) {
	static Intercom_Message msg;
  int rxDataLength = _udp.receivePacket((uint8_t*)&msg, sizeof(msg));
  int payloadSize;
  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[msg.msgId];
  int ii;

  if (rxDataLength < 8)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rxDataLength);
  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %d\n", (int)msg.msgId);

  payloadSize = rxDataLength - 8;

  if (msgEntryp->topIndex==0)
    return -(MODULE_ID+4); /*No handlers for this message*/

  if (msgEntryp->encrypted) {
    if (_decryptMsg(msg, payloadSize) != 0) {
      return -101;
    }
  }

  for (ii=0; ii<msgEntryp->topIndex; ++ii) {
    if (msgEntryp->fun[ii]) {
  	  (*(msgEntryp->fun[ii]))(msg, payloadSize);
    }
  }

  return 0;
}

int Intercom_MessageHandler::_registerHandler(int id, 
  std_function_int_Intercom_MessageRef_int_t func, bool encrypted) {
	plf_assert("Msg ID too large", id <= MAX_MESSAGE_ID);

  auto wrapper = new std_function_int_Intercom_MessageRef_int_t(func);

  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[id];

  plf_assert("Too many handlers", msgEntryp->topIndex<MAX_NUM_FUNS_PER_MSG);

	msgEntryp->fun[msgEntryp->topIndex] = wrapper;
  msgEntryp->encrypted = encrypted;
  ++(msgEntryp->topIndex);

  return 0;
}

Intercom_MessageHandler::Intercom_MessageHandler(int localPort, 
	IPAddress remoteIpAddress, int remotePort) : 
	_remoteIpAddress(remoteIpAddress),
	_remotePort(remotePort), _msgTable(), _myId(ID_UNKNOWN),
  _encryptionKeyIsSet(false) {

	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer\n");
  }

  memset(_ivEnc, 0, sizeof(_ivEnc));
  memset(_ivDec, 0, sizeof(_ivDec));

  mbedtls_xtea_init(&_xteaCtxt);

  PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SECRET_KEY, &Intercom_MessageHandler::_registryHandler, this);

	_udp.begin(localPort);

  dataDump.registerFunction("MessageHandler", &Intercom_MessageHandler::_dataDump, this);
}

void Intercom_MessageHandler::_dataDump(void) {
  PLF_PRINT(PRNTGRP_DFLT, "RemoteIPaddress: %s", _remoteIpAddress.toString().c_str());
  PLF_PRINT(PRNTGRP_DFLT, "RemotePort: %d", _remotePort);
#ifdef MBEDTLS_CIPHER_MODE_CBC
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionMode: CBC");
#else /*ECB:*/
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionMode: ECB");
#endif /*MBEDTLS_CIPHER_MODE*/
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionKeyIsSet: %d", (int)_encryptionKeyIsSet);
}