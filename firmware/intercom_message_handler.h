#ifndef INTERCOM_MESSAGE_HANDLER_H
#define INTERCOM_MESSAGE_HANDLER_H

#include "Particle.h"
#include "xtea.h"
#include <functional>
#include "board.h"

#define MESSAGE_DATA_LENGTH 508
#define MAX_MESSAGE_ID 255
#define ID_UNKNOWN (~0UL)
#define INTERCOM_SERVER_ID 0
#define MAX_NUM_FUNS_PER_MSG 8

typedef struct {
	uint32_t source_id;
	uint32_t rxMsgCounter;
} Intercom_RxMessageCounter;

typedef struct {
	uint32_t destination_id;
	uint32_t txMsgCounter;
} Intercom_TxMessageCounter;

typedef struct {
	uint32_t msgId;
	uint32_t source_id;
	uint8_t data[MESSAGE_DATA_LENGTH];

} Intercom_Message;

extern Intercom_Message intercom_message;

typedef std::function<int (Intercom_Message&, int)> std_function_int_Intercom_MessageRef_int_t;

typedef struct {
	std_function_int_Intercom_MessageRef_int_t *fun[MAX_NUM_FUNS_PER_MSG];
	int topIndex;
	bool encrypted;
} Intercom_MessageHandlerTableElement;

class Intercom_MessageHandler {
private:
	int _localPort;
	UDP _udp;
	Intercom_MessageHandlerTableElement _msgTable[MAX_MESSAGE_ID];
	Intercom_RxMessageCounter _msgRxCounters[NUM_BUDDIES];
	Intercom_TxMessageCounter _msgTxCounters[NUM_BUDDIES];
	mbedtls_xtea_context _xteaCtxt;
	unsigned char _ivEnc[8];
	unsigned char _ivDec[8];
	uint32_t _myId;
	bool _encryptionKeyIsSet;

	void _countRxMsg(Intercom_Message &msg);
	void _countTxMsg(uint32_t destination_id);

	int _encryptMsg(Intercom_Message &msg, int payloadSize);
	int _decryptMsg(Intercom_Message &msg, int payloadSize);

	int _registryHandlerSecretKey(int key, String& value, bool valid);
	int _registryHandlerMyId(int key, String& value, bool valid);

	void _dataDump(void);
	
	int _registerHandler(int id, std_function_int_Intercom_MessageRef_int_t func, bool encrypted);
	
public:
	Intercom_MessageHandler(int localPort);

	uint32_t getMyId(void); /*Returns ID_UNKNOWN if unknown*/

	int send(Intercom_Message &msg, uint32_t msgId, uint32_t destination_id, int payloadSize, IPAddress &serverAddress);
	int receive(void);

	template <typename T>
    int registerHandler(int id, int (T::*func)(Intercom_Message &msg, int payloadSize), T *instance, bool encrypted) {
    	using namespace std::placeholders;
      	return _registerHandler(id, std::bind(func, instance, _1, _2), encrypted);
    }

	void setEncryptionKey(uint8_t key[16]);

	Intercom_RxMessageCounter* allocRxCounter(void);
	void freeRxCounter(Intercom_RxMessageCounter* counter);

	Intercom_TxMessageCounter* allocTxCounter(void);
	void freeTxCounter(Intercom_TxMessageCounter* counter);
};

#endif /*INTERCOM_MESSAGE_HANDLER_H*/