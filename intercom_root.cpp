#include "intercom_root.h"
#include "plf_utils.h"

Intercom_Root::Intercom_Root(void) :
	_message_handler(LOCAL_PORT, REMOTE_IP, REMOTE_PORT, _plf_registry),
	_intercom_incoming(_message_handler),
    _intercom_outgoing(_message_handler),
	_intercom_controller(_message_handler, _plf_registry),
	_intercom_cloud_api(_plf_registry) {
    int ii;

    for (ii=0; ii<NUM_BUDDIES; ++ii) {
        _intercom_buddies[ii].init(&_intercom_outgoing, &_message_handler, &_plf_registry, &_intercomButtonsAndLeds, ii);
    }

	_plf_registry.go();
}

void Intercom_Root::loop(void) {
	int res = _message_handler.receive();
    int ii;

    if (res != 0) {
      PLF_PRINT(PRNTGRP_DFLT, "msg_hdlr rx code %d\n", res);
    }	

    _intercom_incoming.drain();

    if (!_intercom_buddies[0].checkButtonAndSend()) {
        if (!_intercom_buddies[1].checkButtonAndSend()) {
            _intercom_buddies[2].checkButtonAndSend();
        }
    }

    _intercom_controller.tick();

    for (ii=0; ii<NUM_BUDDIES; ++ii) {
        _intercom_buddies[ii].tick();
    }

    _intercom_cloud_api.tick();
}