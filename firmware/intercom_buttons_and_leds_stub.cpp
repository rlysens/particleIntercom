#include "intercom_buttons_and_leds_stub.h"
#include <Particle.h>
#include "plf_utils.h"

#define MODULE_ID 200

void Intercom_LedBar_Stub::setLevel(int level) {
}

void Intercom_LedBar_Stub::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
}

void Intercom_Led_Stub::init(byte pin) {
	_pin = pin;
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
}

void Intercom_Led_Stub::analogWrite(byte iOn) {
	digitalWrite(_pin, iOn!=0 ? HIGH : LOW);
}

void Intercom_Led_Stub::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
	digitalWrite(_pin, HIGH);
}

void Intercom_Led_Stub::breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt, byte offInt) {
	digitalWrite(_pin, HIGH);
}

Intercom_ButtonsAndLeds_Stub::Intercom_ButtonsAndLeds_Stub() : Intercom_ButtonsAndLeds() {
	_leds[0].init(BUDDY_0_LED_DUMMY);
	_leds[1].init(BUDDY_1_LED_DUMMY);

	pinMode(BUDDY_0_BUTTON_DUMMY, INPUT_PULLUP);
	pinMode(BUDDY_1_BUTTON_DUMMY, INPUT_PULLUP);
}

bool Intercom_ButtonsAndLeds_Stub::buttonIsPressed(int buttonId) {
	return false;
}

bool Intercom_ButtonsAndLeds_Stub::buddyButtonIsPressed(int buddyIndex) {
	switch (buddyIndex) {
		case BUDDY_0_BUTTON_DUMMY:
			return digitalRead(BUDDY_0_BUTTON_DUMMY) == LOW;
		case BUDDY_1_BUTTON_DUMMY:
			return digitalRead(BUDDY_1_BUTTON_DUMMY) == LOW;
		default:
			return false;
	}
}

Intercom_Led& Intercom_ButtonsAndLeds_Stub::getBuddyLed(int buddyIndex) {
	plf_assert("getBuddyLed out of range", buddyIndex < NUM_BUDDIES);
	return _leds[buddyIndex];	
}

Intercom_LedBar& Intercom_ButtonsAndLeds_Stub::getLedBar(void) {
	return _ledBar;
}
