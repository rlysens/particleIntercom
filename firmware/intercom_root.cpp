#include "intercom_root.h"
#include "plf_utils.h"
#include "plf_registry.h"

#define MODULE_ID 800

Intercom_Root::Intercom_Root(Intercom_ButtonsAndLeds& intercom_buttonsAndLeds) :
	_messageHandler(LOCAL_PORT),
	_intercom_controller(_messageHandler),
    _intercom_volumeControl(intercom_buttonsAndLeds),
    _intercom_incoming(_messageHandler, _intercom_volumeControl),
    _intercom_batteryChecker(intercom_buttonsAndLeds),
    _intercom_wifiChecker(intercom_buttonsAndLeds),
    _intercom_cloud_api(intercom_buttonsAndLeds, _intercom_wifiChecker, _intercom_batteryChecker) {
    int ii;

    for (ii=0; ii<NUM_BUDDIES; ++ii) {
        _intercom_buddies[ii].init(
            &_intercom_incoming, 
            &_messageHandler, 
            &intercom_buttonsAndLeds,
            ii);
    }

    _intercom_outgoing.init(_messageHandler, _intercom_buddies);

	plf_registry.go();
}

void Intercom_Root::loop(void) {
	int res = _messageHandler.receive();
    int ii;

    if (res != 0) {
      PLF_PRINT(PRNTGRP_DFLT, "msg_hdlr rx code %d\n", res);
    }	

    _intercom_incoming.tick();
    _intercom_incoming.drain();

    _intercom_batteryChecker.checkButton();
    _intercom_batteryChecker.tick();
    
    _intercom_wifiChecker.checkButton();
    
    if (!_intercom_buddies[0].checkButton()) {
        if (!_intercom_buddies[1].checkButton()) {
            _intercom_buddies[2].checkButton();
        }
    }

    _intercom_outgoing.run();
    
    _intercom_volumeControl.checkButtons();
    _intercom_volumeControl.tick();

    _intercom_controller.tick();

    for (ii=0; ii<NUM_BUDDIES; ++ii) {
        _intercom_buddies[ii].tick();
    }

    _intercom_cloud_api.tick();
}