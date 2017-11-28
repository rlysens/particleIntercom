#ifndef INTERCOM_MESSAGE_HANDLER_H
#define INTERCOM_MESSAGE_HANDLER_H

#include "Particle.h"
#include "plf_registry.h"
#include "xtea.h"

#define MESSAGE_DATA_LENGTH 508
#define MAX_MESSAGE_ID 255
#define ID_UNKNOWN (~0UL)
#define MAX_NUM_FUNS_PER_MSG 8
typedef struct {
	uint32_t msg_id;
	uint32_t source_id;
	uint8_t data[MESSAGE_DATA_LENGTH];

} Intercom_Message;

extern Intercom_Message intercom_message;

typedef int (Intercom_MessageHandlerFunType)(Intercom_Message &msg, 
	int payload_size, void *ctxt);

typedef struct {
	Intercom_MessageHandlerFunType *fun[MAX_NUM_FUNS_PER_MSG];
	int topIndex;
	void *ctxt[MAX_NUM_FUNS_PER_MSG];
	bool encrypted;
} Intercom_MessageHandlerTableElement;

class Intercom_MessageHandler {
private:
	IPAddress _remoteIpAddress;
	int _remotePort;
	UDP _udp;
	Intercom_MessageHandlerTableElement _msgTable[MAX_MESSAGE_ID];
	mbedtls_xtea_context _xteaCtxt;
	unsigned char _ivEnc[8];
	unsigned char _ivDec[8];
	uint32_t _myId;
	PlfRegistry& _registry;
	bool _encryptionKeyIsSet;

	int _encryptMsg(Intercom_Message &msg, int payloadSize);
	int _decryptMsg(Intercom_Message &msg, int payloadSize);

public:
	Intercom_MessageHandler(int localPort, IPAddress remoteIpAddress, int remotePort,
		PlfRegistry& registry);

	void setMyId(uint32_t myId);
	uint32_t getMyId(void); /*Returns ID_UNKNOWN if unknown*/

	int send(Intercom_Message &msg, uint32_t msgId, int payloadSize, bool encrypted);
	int receive(void);
	int registerHandler(uint16_t id, Intercom_MessageHandlerFunType *fun,
		void *ctxt, bool encrypted);
	void setEncryptionKey(uint8_t key[16]);
};

#endif /*INTERCOM_MESSAGE_HANDLER_H*/