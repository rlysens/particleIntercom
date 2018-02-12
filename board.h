#ifndef BOARD_H
#define BOARD_H

/*Pins number 1, 2, 3... are on the SX1509. Pins numbered A0, A1, D0, D1... are on the Photon*/

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
#define SX1509_ADDRESS 0x3E  // SX1509 I2C address
#define SX1509_RESET_PIN D2

#define PARTICLE_SDA D0
#define PARTICLE_SCL D1

#define BATTERY_CHECK_BUTTON 0
#define WIFI_CHECK_BUTTON 7

#define NUM_LEDS 3
#define NUM_BUDDIES 3

#define BUDDY_0_LED 4
#define BUDDY_1_LED 5
#define BUDDY_2_LED 6

#define BUDDY_0_BUTTON_DUMMY D0
#define BUDDY_1_BUTTON_DUMMY D1
#define BUDDY_0_LED_DUMMY D3
#define BUDDY_1_LED_DUMMY D4

#define BUDDY_0_BUTTON 1
#define BUDDY_1_BUTTON 2
#define BUDDY_2_BUTTON 3

#define VOL_DEC_BUTTON 8
#define VOL_INC_BUTTON 9

#define LED_BAR_0_LED 10
#define LED_BAR_1_LED 11
#define LED_BAR_2_LED 12
#define LED_BAR_3_LED 13
#define LED_BAR_4_LED 14

#define INTERCOM_CODEC_XCS DAC
#define INTERCOM_CODEC_SI A5
#define INTERCOM_CODEC_SO A4
#define INTERCOM_CODEC_XDCS A2
#define INTERCOM_CODEC_XRESET A1
#define INTERCOM_CODEC_DREQ A0

#endif /*BOARD_H*/