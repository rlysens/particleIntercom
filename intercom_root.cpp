#include "intercom_root.h"
#include "plf_utils.h"

#define MODULE_ID 800

Intercom_Root::Intercom_Root(void) :
	_messageHandler(LOCAL_PORT, REMOTE_IP, REMOTE_PORT, _plf_registry),
	_intercom_incoming(_messageHandler),
    _intercom_outgoing(_messageHandler),
	_intercom_controller(_messageHandler, _plf_registry),
	_intercom_cloud_api(_plf_registry) {
    int ii;

    for (ii=0; ii<NUM_BUDDIES; ++ii) {
        _intercom_buddies[ii].init(&_intercom_outgoing, &_messageHandler, &_plf_registry, &_intercom_buttonsAndLeds, ii);
    }

	_plf_registry.go();
}

void Intercom_Root::loop(void) {
	int res = _messageHandler.receive();
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