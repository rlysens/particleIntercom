#include "intercom_buttons_and_leds.h"
#include <Particle.h>
#include "plf_utils.h"

#define MODULE_ID 200

Intercom_LedBar::Intercom_LedBar() : _iop(0) {
}

void Intercom_LedBar::init(SX1509& io, byte pins[LED_BAR_MAX_LEVEL]) {
	int ii;

	_iop = &io;

	for (ii=0;ii<LED_BAR_MAX_LEVEL; ii++) {
		_pins[ii] = pins[ii];
		_iop->ledDriverInit(pins[ii]);
	}

	setLevel(0);
}

void Intercom_LedBar::setLevel(int level) {
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

Intercom_Led::Intercom_Led() : _iop(0), _pin(255) {
}

void Intercom_Led::init(SX1509& io, byte pin) {
	_iop = &io;
	_pin = pin;
	_iop->ledDriverInit(pin);
	analogWrite(0);
}

void Intercom_Led::analogWrite(byte iOn) {
	plf_assert("Intercom_Led io=0", _iop);

	_iop->analogWrite(_pin, iOn);
#if 0
	_iop->breathe(_pin, 100, 0, 0, 0, iOn, iOn);
#endif
}

void Intercom_Led::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
	plf_assert("Intercom_Led io=0", _iop);
	_iop->blink(_pin, tOn, tOff, onIntensity, offIntensity);
}

void Intercom_Led::breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt, byte offInt) {
	plf_assert("Intercom_Led io=0", _iop);
	_iop->breathe(_pin, tOn, tOff, rise, fall, onInt, offInt);
}

Intercom_ButtonsAndLeds::Intercom_ButtonsAndLeds() {
	byte result;
	static byte pins[] = {LED_BAR_0_LED,LED_BAR_1_LED,LED_BAR_2_LED,LED_BAR_3_LED,LED_BAR_4_LED};

	result = _io.begin(SX1509_ADDRESS, SX1509_RESET_PIN);
	plf_assert("SX1509 init. failed", result!=0);

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
}

void Intercom_ButtonsAndLeds::reset(void) {
	_io.reset(true /*hardware*/);
}

bool Intercom_ButtonsAndLeds::buttonIsPressed(int buttonId) {
	return _io.digitalRead(buttonId) == LOW;
}

bool Intercom_ButtonsAndLeds::buddyButtonIsPressed(int buddyIndex) {
	plf_assert("buddyButtonIsPressed out of range", buddyIndex < NUM_BUDDIES);
	static const byte pin[NUM_BUDDIES] = {BUDDY_0_BUTTON, BUDDY_1_BUTTON, BUDDY_2_BUTTON};
	return _io.digitalRead(pin[buddyIndex]) == LOW;
}

Intercom_Led& Intercom_ButtonsAndLeds::getBuddyLed(int buddyIndex) {
	plf_assert("getBuddyLed out of range", buddyIndex < NUM_BUDDIES);
	return _leds[buddyIndex];	
}

Intercom_LedBar& Intercom_ButtonsAndLeds::getLedBar(void) {
	return _ledBar;
}
