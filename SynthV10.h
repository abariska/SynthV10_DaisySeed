#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "SX1509_extender.h"
#include "daisy_core.h"


using namespace daisy;
using namespace daisysp;

// Global objects
extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;

// Function prototypes
void ProcessButtons();
void ProcessEncoders();
void ProcessLeds();
void RawPinState();
void TimersInit();


#endif