#include "intercom_message_handler.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "plf_data_dump.h"
#include "plf_registry.h"
#include "message_name_table.h"
#include "messages.h"

#define MODULE_ID 600

Intercom_Message intercom_message; /*Shared by all message_handler users*/

int Intercom_MessageHandler::_registryHandlerSecretKey(int key) {
  String value;
  bool valid;

  plf_registry.getString(key, value, valid);

  if (valid) {
    uint8_t keyArray[17]; /*+1 because getBytes add zero terminator*/
    value.getBytes(keyArray, sizeof(keyArray));
    setEncryptionKey(keyArray);
  }

  return 0;
}

int Intercom_MessageHandler::_registryHandlerMyId(int key) {
  int value = ID_UNKNOWN;
  bool valid;

  plf_registry.getInt(key, value, valid);
  _myId = value;

  return 0;
}

void Intercom_MessageHandler::setEncryptionKey(uint8_t key[16]) {
  _encryptionKeyIsSet = true;
  mbedtls_xtea_setup( &_xteaCtxt, key );
}

Intercom_RxMessageCounter* Intercom_MessageHandler::allocRxCounter(void) {
  int ii;

  for (ii=0;ii<NUM_BUDDIES;ii++) {
    if (_msgRxCounters[ii].source_id == 0) {
      _msgRxCounters[ii].source_id = ID_UNKNOWN;
      return &_msgRxCounters[ii];
    }
  }

  return 0;
}

void Intercom_MessageHandler::freeRxCounter(Intercom_RxMessageCounter* counter) {
  plf_assert("counter ptr NULL", counter);
  counter->source_id = 0;  
}

void Intercom_MessageHandler::_countRxMsg(Intercom_Message &msg) {
  int ii;
  uint32_t source_id = msg.source_id;

  if (source_id > INTERCOM_SERVER_ID) {
    for (ii=0; ii<NUM_BUDDIES; ii++) {
      if (source_id == _msgRxCounters[ii].source_id) {
        ++(_msgRxCounters[ii].rxMsgCounter);
        return;
      }
    }    
  }
}

Intercom_TxMessageCounter* Intercom_MessageHandler::allocTxCounter(void) {
  int ii;

  for (ii=0;ii<NUM_BUDDIES;ii++) {
    if (_msgTxCounters[ii].destination_id == 0) {
      _msgTxCounters[ii].destination_id = ID_UNKNOWN;
      return &_msgTxCounters[ii];
    }
  }

  return 0;
}

void Intercom_MessageHandler::freeTxCounter(Intercom_TxMessageCounter* counter) {
  plf_assert("counter ptr NULL", counter);
  counter->destination_id = 0;  
}

void Intercom_MessageHandler::_countTxMsg(uint32_t destination_id) {
  int ii;

  if (destination_id > INTERCOM_SERVER_ID) {
    for (ii=0; ii<NUM_BUDDIES; ii++) {
      if (destination_id == _msgTxCounters[ii].destination_id) {
        ++(_msgTxCounters[ii].txMsgCounter);
        return;
      }
    }    
  }
}

int Intercom_MessageHandler::_encryptMsg(Intercom_Message &msg, int payloadSize) {
  int result=-1;

  if (_encryptionKeyIsSet) {
#if MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_CBC
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_ENCRYPT,
      payloadSize,
      _ivEnc,
      msg.data,
      msg.data);
#elif MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_ECB
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
#else /*CIPHER_MODE_NONE*/
    result=0;
#endif /*MBEDTLS_CIPHER_MODE_CBC*/
  }

  return result;
}

int Intercom_MessageHandler::_decryptMsg(Intercom_Message &msg, int payloadSize) {
  int result=-(MODULE_ID+1);

  if (_encryptionKeyIsSet) {
#if MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_CBC
    result = mbedtls_xtea_crypt_cbc(&_xteaCtxt,
      MBEDTLS_XTEA_DECRYPT,
      payloadSize,
      _ivDec,
      msg.data,
      msg.data);
#elif MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_ECB
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
#else /*CIPHER_MODE_NONE*/
    result = 0;
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

uint32_t Intercom_MessageHandler::getMyId(void) {
  return _myId;
}

int Intercom_MessageHandler::send(Intercom_Message &msg, uint32_t msgId, uint32_t destination_id, int payloadSize, IPAddress& serverAddress) {
  PLF_PRINT(PRNTGRP_MSGS, "Tx Msg %s(%d), dest: %d\n", 
    msgId < sizeof(messageNameTable) ? messageNameTable[msgId] : "X", (int)msgId, (int)destination_id);

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
  
  if (_encryptMsg(msg, payloadSize)!=0) {
    return -(MODULE_ID+2);
  }

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

  if (!serverAddress) {
    return -(MODULE_ID+7);
  }

  /* Send the UDP packet */
  int res = _udp.sendPacket((uint8_t*)&msg, payloadSize+8, 
      serverAddress, 
      _localPort /*Use same port number as local port for remote port.*/);
  if (res != payloadSize+8) {
      PLF_PRINT(PRNTGRP_DFLT, "UDP packet send failed. Could not send all data: %d", res);
      _udp.begin(_localPort); /*Socket closes on error. Reopen it.*/
      return -(MODULE_ID+3);
  }

  PLF_COUNT_VAL(UDP_BYTES_TX, payloadSize);
  
  _countTxMsg(destination_id);

  return 0;
}

int Intercom_MessageHandler::receive(void) {
	static Intercom_Message msg;
  int rxDataLength = _udp.receivePacket((uint8_t*)&msg, sizeof(msg));
  int payloadSize;
  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[msg.msgId];
  int ii;

  #if 0
  if (rxDataLength < 0) {
    PLF_PRINT(PRNTGRP_DFLT, "UDP packet receive failed: %d", rxDataLength);
    _udp.begin(_localPort); /*Socket closes on error. Reopen it.*/
    return -(MODULE_ID+5);
  }
  #endif
  
  if (rxDataLength < 8)
    return 0;

  PLF_COUNT_VAL(UDP_BYTES_RX, rxDataLength);
  PLF_PRINT(PRNTGRP_MSGS, "Rx Msg %s(%d), src:%d", 
    msg.msgId < sizeof(messageNameTable) ? messageNameTable[msg.msgId] : "X", (int)msg.msgId, (int)msg.source_id);

  payloadSize = rxDataLength - 8;

  if (msgEntryp->topIndex==0)
    return -(MODULE_ID+4); /*No handlers for this message*/

  if (msgEntryp->encrypted) {
    if (_decryptMsg(msg, payloadSize) != 0) {
      return -(MODULE_ID+6);
    }
  }

  _countRxMsg(msg);

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

  plf_assert("_registerHandler new fail", wrapper!=NULL);
  
  Intercom_MessageHandlerTableElement *msgEntryp = &_msgTable[id];

  plf_assert("Too many handlers", msgEntryp->topIndex<MAX_NUM_FUNS_PER_MSG);

	msgEntryp->fun[msgEntryp->topIndex] = wrapper;
  msgEntryp->encrypted = encrypted;
  ++(msgEntryp->topIndex);

  return 0;
}

Intercom_MessageHandler::Intercom_MessageHandler(int localPort) : 
	_localPort(localPort), _msgTable(), _myId(ID_UNKNOWN),
  _encryptionKeyIsSet(false) {
	if (!_udp.setBuffer(sizeof(Intercom_Message))) {
      PLF_PRINT(PRNTGRP_DFLT, "Couldn't allocate outgoing packet buffer.");
  }

  memset(_msgRxCounters, 0, sizeof(_msgRxCounters));
  memset(_msgTxCounters, 0, sizeof(_msgTxCounters));
  memset(_ivEnc, 0, sizeof(_ivEnc));
  memset(_ivDec, 0, sizeof(_ivDec));

  mbedtls_xtea_init(&_xteaCtxt);

  PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SECRET_KEY, &Intercom_MessageHandler::_registryHandlerSecretKey, this);
  PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_MY_ID, &Intercom_MessageHandler::_registryHandlerMyId, this);

	_udp.begin(localPort);

  dataDump.registerFunction("MessageHandler", &Intercom_MessageHandler::_dataDump, this);
}

void Intercom_MessageHandler::_dataDump(void) {
#if MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_CBC
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionMode: CBC");
#elif MBEDTLS_CIPHER_MODE==MBEDTLS_CIPHER_MODE_ECB
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionMode: ECB");
#else
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionMode: None");
#endif /*MBEDTLS_CIPHER_MODE*/
  PLF_PRINT(PRNTGRP_DFLT, "EncryptionKeyIsSet: %d", (int)_encryptionKeyIsSet);
  PLF_PRINT(PRNTGRP_DFLT, "MyId: %d", (int)_myId);

  int ii;
  for (ii=0; ii<NUM_BUDDIES; ii++) {
    PLF_PRINT(PRNTGRP_DFLT, "RxMsgCounters[%d].source_id = %d, rxMsgCounter=%d",
      ii, (int)_msgRxCounters[ii].source_id, (int)_msgRxCounters[ii].rxMsgCounter);
  }

  for (ii=0; ii<NUM_BUDDIES; ii++) {
    PLF_PRINT(PRNTGRP_DFLT, "TxMsgCounters[%d].destination_id = %d, txMsgCounter=%d",
      ii, (int)_msgTxCounters[ii].destination_id, (int)_msgTxCounters[ii].txMsgCounter);
  }
}