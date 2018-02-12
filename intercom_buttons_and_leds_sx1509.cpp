#include "intercom_buttons_and_leds_sx1509.h"
#include <Particle.h>
#include "plf_utils.h"

#define MODULE_ID 200

Intercom_LedBar_SX1509::Intercom_LedBar_SX1509() : Intercom_LedBar(), _iop(0) {
}

void Intercom_LedBar_SX1509::init(SX1509& io, byte pins[LED_BAR_MAX_LEVEL]) {
	int ii;

	_iop = &io;

	for (ii=0;ii<LED_BAR_MAX_LEVEL; ii++) {
		_pins[ii] = pins[ii];
		_iop->ledDriverInit(pins[ii]);
	}

	setLevel(0);
}

void Intercom_LedBar_SX1509::setLevel(int level) {
	int ii;

	plf_assert("Led bar level out of range.", level <= LED_BAR_MAX_LEVEL);
	plf_assert("Led bar level out of range.", level >= 0);
	plf_assert("Led bar io=0", _iop);

	for (ii=0; ii<level; ++ii) {
		_iop->analogWrite(_pins[ii], 255);
	}

	for (ii=level; ii<LED_BAR_MAX_LEVEL; ++ii) {
		_iop->analogWrite(_pins[ii], 0);
	}
}

Intercom_Led_SX1509::Intercom_Led_SX1509() : Intercom_Led(), _iop(0), _pin(255) {
}

void Intercom_Led_SX1509::init(SX1509& io, byte pin) {
	_iop = &io;
	_pin = pin;
	_iop->ledDriverInit(pin);
	analogWrite(0);
}

void Intercom_Led_SX1509::analogWrite(byte iOn) {
	plf_assert("Intercom_Led_SX1509 io=0", _iop);

	_iop->analogWrite(_pin, iOn);
}

void Intercom_Led_SX1509::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
	plf_assert("Intercom_Led_SX1509 io=0", _iop);
	_iop->blink(_pin, tOn, tOff, onIntensity, offIntensity);
}

void Intercom_Led_SX1509::breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt, byte offInt) {
	plf_assert("Intercom_Led_SX1509 io=0", _iop);
	_iop->breathe(_pin, tOn, tOff, rise, fall, onInt, offInt);
}

Intercom_ButtonsAndLeds_SX1509::Intercom_ButtonsAndLeds_SX1509() : Intercom_ButtonsAndLeds() {
	byte result;
	static byte pins[] = {LED_BAR_0_LED,LED_BAR_1_LED,LED_BAR_2_LED,LED_BAR_3_LED,LED_BAR_4_LED};

	/*Take the SX1509 out of reset*/
	pinMode(SX1509_RESET_PIN, OUTPUT);
	digitalWrite(SX1509_RESET_PIN, HIGH);

	result = _io.begin(SX1509_ADDRESS);
	while (result==0) {
		PLF_PRINT(PRNTGRP_DFLT, "SX1509 init. failed.\n");
		result = _io.begin(SX1509_ADDRESS);
	}

	_leds[BUDDY_0_IDX].init(_io, BUDDY_0_LED);
	_leds[BUDDY_1_IDX].init(_io, BUDDY_1_LED);
	_leds[BUDDY_2_IDX].init(_io, BUDDY_2_LED);

	_ledBar.init(_io, pins);

	_io.pinMode(BATTERY_CHECK_BUTTON, INPUT_PULLUP);
	_io.pinMode(WIFI_CHECK_BUTTON, INPUT_PULLUP);


	_io.pinMode(BUDDY_0_BUTTON, INPUT_PULLUP);
	_io.pinMode(BUDDY_1_BUTTON, INPUT_PULLUP);
	_io.pinMode(BUDDY_2_BUTTON, INPUT_PULLUP);

	_io.pinMode(VOL_DEC_BUTTON, INPUT_PULLUP);
	_io.pinMode(VOL_INC_BUTTON, INPUT_PULLUP);

	_io.debounceTime(32 /*ms*/);

	if (result==0) {
		int ii;
		for (ii=0;ii<NUM_LEDS;ii++) {
			_leds[ii].blink(255, 0);
		}

		while (1);
	}
}

bool Intercom_ButtonsAndLeds_SX1509::buttonIsPressed(int buttonId) {
	return _io.digitalRead(buttonId) == LOW;
}

bool Intercom_ButtonsAndLeds_SX1509::buddyButtonIsPressed(int buddyIndex) {
	plf_assert("buddyButtonIsPressed out of range", buddyIndex < NUM_BUDDIES);
	static const byte pin[NUM_BUDDIES] = {BUDDY_0_BUTTON, BUDDY_1_BUTTON, BUDDY_2_BUTTON};
	return _io.digitalRead(pin[buddyIndex]) == LOW;
}

Intercom_Led& Intercom_ButtonsAndLeds_SX1509::getBuddyLed(int buddyIndex) {
	plf_assert("getBuddyLed out of range", buddyIndex < NUM_BUDDIES);
	return _leds[buddyIndex];	
}

Intercom_LedBar& Intercom_ButtonsAndLeds_SX1509::getLedBar(void) {
	return _ledBar;
}
