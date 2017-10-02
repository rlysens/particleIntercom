#include "intercom_root.h"
#include "plf_utils.h"
#include "intercom_buttons.h"

Intercom_Root::Intercom_Root(void) :
	_message_handler(LOCAL_PORT, REMOTE_IP, REMOTE_PORT),
	_intercom_incoming(_message_handler),
	_intercom_outgoing(_message_handler),
	_intercom_controller(_message_handler, _intercom_outgoing, _plf_registry),
	_intercom_cloud_api(_plf_registry) {

	_plf_registry.go();
}

void Intercom_Root::loop(void) {
	int res = _message_handler.receive();
    if (res != 0) {
      PLF_PRINT(PRNTGRP_DFLT, "msg_hdlr rx code %d\n", res);
    }	

    _intercom_incoming.drain();

    if (recordButtonPressed()) {
    	_intercom_outgoing.transfer();
    }

    _intercom_controller.tick();
}