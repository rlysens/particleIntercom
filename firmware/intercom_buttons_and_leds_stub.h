#ifndef INTERCOM_BUTTONS_AND_LEDS_STUB_H
#define INTERCOM_BUTTONS_AND_LEDS_STUB_H

#include "intercom_buttons_and_leds.h"

class Intercom_Led_Stub : public Intercom_Led {
private:
	byte _pin;

public:
	void init(byte pin);

	// -----------------------------------------------------------------------------
	// analogWrite(byte iOn):	This function can be used to control the intensity 
	//		of an output pin connected to an LED.
	//
	//	Inputs:
	//		- iOn: should be a 0-255 value setting the intensity of the LED
	//			- 0 is completely off, 255 is 100% on.
	// -----------------------------------------------------------------------------
	virtual void analogWrite(byte iOn);

	// -----------------------------------------------------------------------------
	// blink(unsigned long tOn, unsigned long tOff, byte onIntensity, byte offIntensity);
	//  	Set a pin to blink output for estimated on/off millisecond durations.
	//
	// 	Inputs:
	//   	- tOn: estimated number of milliseconds the pin is LOW (LED sinking current will be on)
	//   	- tOff: estimated number of milliseconds the pin is HIGH (LED sinking current will be off)
	//   	- onIntensity: 0-255 value determining LED on brightness
	//   	- offIntensity: 0-255 value determining LED off brightness
	// 	 Notes: 
	// -----------------------------------------------------------------------------
	virtual void blink(unsigned long tOn, unsigned long tOff, byte onIntensity = 255, byte offIntensity = 0);

	// -----------------------------------------------------------------------------
	// breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, byte onInt, byte offInt, bool log);
	//  	Set a pin to breathe output for estimated on/off millisecond durations, with
	//  	estimated rise and fall durations.
	//
	// 	Inputs:
	//   	- tOn: estimated number of milliseconds the pin is LOW (LED sinking current will be on)
	//   	- tOff: estimated number of milliseconds the pin is HIGH (LED sinking current will be off)
	//   	- rise: estimated number of milliseconds the pin rises from LOW to HIGH
	//   	- fall: estimated number of milliseconds the pin falls from HIGH to LOW
	//   	- onIntensity: 0-255 value determining LED on brightness
	//   	- offIntensity: 0-255 value determining LED off brightness
	// -----------------------------------------------------------------------------
	virtual void breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt = 255, byte offInt = 0);
};

class Intercom_ButtonsAndLeds_Stub: public Intercom_ButtonsAndLeds {
private:
	Intercom_Led_Stub _leds[NUM_LEDS];
	Intercom_LedBar _ledBar;

public:
	Intercom_ButtonsAndLeds_Stub();

	virtual bool buddyButtonIsPressed(int buddyIndex);
	virtual Intercom_Led& getBuddyLed(int buddyIndex);
	virtual Intercom_LedBar& getLedBar(void);
};

#endif /*INTERCOM_BUTTONS_AND_LEDS_STUB_H*/