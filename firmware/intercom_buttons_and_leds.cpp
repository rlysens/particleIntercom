#include "intercom_buttons_and_leds.h"
#include "plf_utils.h"

bool Intercom_LedBar::setExclusive(bool enable) {
	if (enable) {
		if (!_exclusive) {
			_exclusive = true;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		_exclusive = false;
		return false;
	}
}

bool Intercom_LedBar::isExclusive(void) {
	PLF_PRINT(PRNTGRP_DFLT, "isExclusive: %d\n", _exclusive);
	return _exclusive;
}
	
void Intercom_LedBar::reset(void) {
}

void Intercom_LedBar::setLevel(int level) {
}

void Intercom_LedBar::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
}

void Intercom_LedBar::breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall,
		unsigned long startIdx, unsigned long stopIdx, byte onInt, byte offInt) {
}

Intercom_LedBar::Intercom_LedBar() : _exclusive(false) {
}

void Intercom_Led::analogWrite(byte iOn) {
}

void Intercom_Led::blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity) {
}

void Intercom_Led::breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt, byte offInt) {
}

bool Intercom_ButtonsAndLeds::buttonIsPressed(int buttonId) {
	return false;
}

bool Intercom_ButtonsAndLeds::buddyButtonIsPressed(int buddyIndex) {
	return false;
}
	