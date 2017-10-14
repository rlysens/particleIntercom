#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "Particle.h"
#include "message_handler.h"
#include "xtea.h"

#define MESSAGE_DATA_LENGTH 508
#define MAX_MESSAGE_ID 255

typedef struct {
	uint32_t msg_id;
	uint32_t source_id;
	uint8_t data[MESSAGE_DATA_LENGTH];

} Intercom_Message;

typedef int (MessageHandlerFunType)(Intercom_Message &msg, 
	int payload_size, void *ctxt);

typedef struct {
	MessageHandlerFunType *fun;
	void *ctxt;
	bool encrypted;
} MessageHandlerTableElement;

class Message_Handler {
private:
	IPAddress _remote_ip_address;
	int _remote_port;
	UDP _udp;
	MessageHandlerTableElement _msgTable[MAX_MESSAGE_ID];
	mbedtls_xtea_context _xtea_ctxt;
	unsigned char _iv_enc[8];
	unsigned char _iv_dec[8];
	uint32_t _source_id;

	void _encrypt_msg(Intercom_Message &msg, int payload_size);
	void _decrypt_msg(Intercom_Message &msg, int payload_size);

public:
	Message_Handler(int local_port, IPAddress remote_ip_address, int remote_port);

	void set_source_id(uint32_t source_id);
	void set_encryption_key(uint8_t key[16]);
	int send(Intercom_Message &msg, uint32_t msg_id, int payload_size, bool encrypted);
	int receive(void);
	int register_handler(uint16_t id, MessageHandlerFunType *fun,
		void *ctxt, bool encrypted);
};

#endif /*MESSAGE_HANDLER_H*/