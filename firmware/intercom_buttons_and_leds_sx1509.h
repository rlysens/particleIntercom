#ifndef INTERCOM_BUTTONS_AND_LEDS_SX1509_H
#define INTERCOM_BUTTONS_AND_LEDS_WX1509_H

#include "intercom_buttons_and_leds.h"
#include "SparkFunSX1509.h" // Include SX1509 library

class Intercom_LedBar_SX1509 : public Intercom_LedBar {
private:
	SX1509* _iop;
	byte _pins[LED_BAR_NUM_LEDS];

public:
	Intercom_LedBar_SX1509();

	virtual void reset(void);
	virtual void setLevel(int level);

	virtual void blink(unsigned long tOn, unsigned long tOff, byte onIntensity = 255, byte offIntensity = 0);

	virtual void breathe(unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall,
		unsigned long startIdx=0, unsigned long stopIdx=LED_BAR_NUM_LEDS, 
		byte onInt = 255, byte offInt = 0);

	/*private*/
	void init(SX1509& io, byte pins[LED_BAR_NUM_LEDS]);
};

class Intercom_Led_SX1509 : public Intercom_Led {
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

	/*private*/
	void init(SX1509& io, byte pin);

	Intercom_Led_SX1509();
};

class Intercom_ButtonsAndLeds_SX1509 : public Intercom_ButtonsAndLeds {
private:
	SX1509 _io; 
	Intercom_Led_SX1509 _leds[NUM_LEDS];
	Intercom_LedBar_SX1509 _ledBar;

public:
	/*Button IDs: WIFI_CHECK_BUTTON, BATTERY_CHECK_BUTTON, VOL_INC_BUTTON, VOL_DEC_BUTTON. See board.h*/
	virtual bool buttonIsPressed(int buttonId);

	virtual bool buddyButtonIsPressed(int buddyIndex);
	virtual Intercom_Led& getBuddyLed(int buddyIndex);
	virtual Intercom_LedBar& getLedBar(void);

	void reset(void);

	Intercom_ButtonsAndLeds_SX1509();
};

#endif /*INTERCOM_BUTTONS_AND_LEDS_SX1509_H*/