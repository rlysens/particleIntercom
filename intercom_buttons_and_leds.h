#ifndef INTERCOM_BUTTONS_AND_LEDS_H
#define INTERCOM_BUTTONS_AND_LEDS_H

#include "board.h"
#include "SparkFunSX1509.h" // Include SX1509 library

#define BUDDY_0_IDX 0
#define BUDDY_1_IDX 1
#define BUDDY_2_IDX 2

#define LED_BAR_MAX_LEVEL 5

class Intercom_LedBar {
private:
	SX1509* _iop;
	byte _pins[LED_BAR_MAX_LEVEL];

public:
	Intercom_LedBar();

	void setLevel(int level);

	/*private*/
	void init(SX1509& io, byte pins[LED_BAR_MAX_LEVEL]);
};

class Intercom_Led {
private:
	SX1509* _iop;
	byte _pin;

public:
	// -----------------------------------------------------------------------------
	// analogWrite(byte iOn):	This function can be used to control the intensity 
	//		of an output pin connected to an LED.
	//
	//	Inputs:
	//		- iOn: should be a 0-255 value setting the intensity of the LED
	//			- 0 is completely off, 255 is 100% on.
	// -----------------------------------------------------------------------------
	void analogWrite(byte iOn);

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
	void blink(unsigned long tOn, unsigned long tOff, byte onIntensity = 255, byte offIntensity = 0);

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
	void breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, 
		byte onInt = 255, byte offInt = 0);

	/*private*/
	void init(SX1509& io, byte pin);

	Intercom_Led();
};

class Intercom_ButtonsAndLeds {
private:
	SX1509 _io; 
	Intercom_Led _leds[NUM_LEDS];
	Intercom_LedBar _ledBar;
public:
	bool batteryCheckButtonIsPressed(void);
	bool incVolumeButtonIsPressed(void);
	bool decVolumeButtonIsPressed(void);

	bool buddyButtonIsPressed(int buddyIndex);
	Intercom_Led& getBuddyLed(int buddyIndex);
	Intercom_LedBar& getLedBar(void);

	void reset(void);

	Intercom_ButtonsAndLeds();
};

#endif /*INTERCOM_BUTTONS_AND_LEDS_H*/