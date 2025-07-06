#ifndef SX1509_EXTENDER_H
#define SX1509_EXTENDER_H

#include "SX1509_Daisy_Seed/SX1509.h"
#include "daisy_seed.h"

// sx1509 buttons
#define BUTTON_OSC_1 0
#define BUTTON_OSC_2 1
#define BUTTON_OSC_3 2
#define BUTTON_FLT 3
#define BUTTON_AMP 4
#define BUTTON_LFO 5
#define BUTTON_MTX 6
#define BUTTON_FX 7
#define BUTTON_STORE 8
#define BUTTON_SHIFT 9
#define BUTTON_BACK 10
#define ENC_1_SW 11
#define ENC_2_SW 12
#define ENC_3_SW 13
#define ENC_4_SW 14
#define ENC_DIAL_SW 15

// sx1509 encoders
#define ENC_1_B 0
#define ENC_1_A 1
#define ENC_2_B 2
#define ENC_2_A 3
#define ENC_3_B 4
#define ENC_3_A 5
#define ENC_4_B 6
#define ENC_4_A 7
#define ENC_DIAL_B 8
#define ENC_DIAL_A 9

// sx1509 leds
#define LED_OSC_1 0
#define LED_OSC_2 1
#define LED_OSC_3 2
#define LED_FLT 3
#define LED_AMP 4
#define LED_LFO 5
#define LED_MTX 6
#define LED_FX 7
#define LED_SHIFT 8
#define LED_BACK 9

extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;

void InitSX1509Extenders();



#endif