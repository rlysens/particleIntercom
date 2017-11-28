#include "intercom_buttons_and_leds.h"

#include <Particle.h>
#include "plf_utils.h"
#include "intercom_buddy.h"

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
#define SX1509_ADDRESS 0x3E  // SX1509 I2C address
#define SX1509_RESET_PIN D0

#define BUDDY_0_LED 4
#define BUDDY_1_LED 5
#define BUDDY_2_LED 6

#define BUDDY_0_BUTTON 1
#define BUDDY_1_BUTTON 2
#define BUDDY_2_BUTTON 3

#define VOL_DEC_BUTTON 8
#define VOL_INC_BUTTON 9

Intercom_Led::Intercom_Led() : _iop(0), _pin(255) {
}

void Intercom_Led::init(SX1509& io, byte pin) {
	_iop = &io;
	_pin = pin;
	_iop->ledDriverInit(pin);
}

void Intercom_Led::analogWrite(byte iOn) {
	plf_assert("Intercom_Led io=0", _iop);
	_iop->analogWrite(_pin, iOn);
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

	result = _io.begin(SX1509_ADDRESS, SX1509_RESET_PIN);
	plf_assert("SX1509 init. failed", result!=0);

	_leds[BUDDY_0_IDX].init(_io, BUDDY_0_LED);
	_leds[BUDDY_1_IDX].init(_io, BUDDY_1_LED);
	_leds[BUDDY_2_IDX].init(_io, BUDDY_2_LED);

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

bool Intercom_ButtonsAndLeds::incVolumeButtonIsPressed(void) {
	return _io.digitalRead(VOL_INC_BUTTON) == LOW;
}

bool Intercom_ButtonsAndLeds::decVolumeButtonIsPressed(void) {
	return _io.digitalRead(VOL_DEC_BUTTON) == LOW;
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
