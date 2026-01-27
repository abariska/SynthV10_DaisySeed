#ifndef SX1509_EXTENDER_H
#define SX1509_EXTENDER_H

#include <stdint.h>
#include "daisy_seed.h"
#include "SX1509_Daisy_Seed/SX1509.h"

// sx1509 buttons
#define BUTTON_OSC_3 2
#define BUTTON_OSC_2 1
#define BUTTON_OSC_1 0
#define BUTTON_FLT 3
#define BUTTON_AMP 4
#define BUTTON_LFO 5
#define BUTTON_MTX 6
#define BUTTON_SETTINGS 7
#define BUTTON_STORE 8
#define BUTTON_EXIT 9
#define BUTTON_FX 10
#define BUTTON_SHIFT 11
#define ENC_1_SW 12
#define ENC_2_SW 13
#define ENC_3_SW 14
#define ENC_4_SW 15

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
#define ENC_DIAL_SW 10

// sx1509 leds
#define LED_OSC_1 2
#define LED_OSC_2 1
#define LED_OSC_3 0
#define LED_LFO 3
#define LED_STORE 4
#define LED_1 5
#define LED_2 6
#define LED_3 7
#define LED_4 8
#define LED_5 9
#define LED_6 10

#define NUM_BUTTONS 16
#define NUM_ENCODERS 5
#define NUM_LEDS 8
extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;

void InitSX1509Extenders();
int8_t EncoderInc(uint8_t pin_a, uint8_t pin_b);
void UpdateLeds();
void UpdatePWMLeds();
void UpdateStoreLed();

#endif