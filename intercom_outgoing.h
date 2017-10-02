#ifndef INTERCOM_PROXY_H
#define INTERCOM_PROXY_H

#include "Particle.h"
#include "plf_utils.h"
#include "message_handler.h"

class Intercom_Outgoing {
private:
	Message_Handler& _message_handler;
	int16_t _destination_id;
	bool _destination_id_set;
	int16_t _source_id;
	bool _source_id_set;
	
public:
  	Intercom_Outgoing(Message_Handler& message_handler);
  	void transfer(void);
  	void set_destination_id(int16_t destination_id);
  	void set_source_id(int16_t source_id);
};

#endif /*INTERCOM_PROXY_H*/
