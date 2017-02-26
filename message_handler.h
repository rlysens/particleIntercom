#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "Particle.h"
#include "message_handler.h"

#define MESSAGE_DATA_LENGTH 510
#define MAX_MESSAGE_ID 255

typedef struct {
	uint16_t id;
	uint8_t data[MESSAGE_DATA_LENGTH];

} Message;

typedef int (MessageHandlerFunType)(Message &msg, 
	int payload_size, void *ctxt);

typedef struct {
	MessageHandlerFunType *fun;
	void *ctxt;
} MessageHandlerTableElement;

class Message_Handler {
private:
	IPAddress _remote_ip_address;
	int _remote_port;
	UDP _udp;
	MessageHandlerTableElement _msgTable[MAX_MESSAGE_ID];

public:
	Message_Handler(int local_port, IPAddress remote_ip_address, int remote_port);

	int send(Message &msg, int payload_size);
	int receive(void);
	int register_handler(uint16_t id, MessageHandlerFunType *fun,
		void *ctxt);
};

#endif /*MESSAGE_HANDLER_H*/