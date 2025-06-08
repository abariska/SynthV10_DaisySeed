#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "Parameters.h"
#include "SX1509_extender.h"
#include "Voice.h"
#include "Menu.h"
#include "Effects.h"
#include "MidiHandler.h"
#include "daisy_core.h"


using namespace daisy;
using namespace daisysp;

// Global objects
extern VoiceUnit voice[NUM_VOICES];
extern MidiUsbHandler midi;
extern CpuLoadMeter cpu_load;
extern FXSlot effectSlot[2];
extern SynthParams params;
extern int encoderIncs[4];
extern bool isParamEditMode[4];
extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;

// Function prototypes
void DisplayView(void* data);
void TimerDisplay();
void ProcessButtons();
void ProcessEncoders();
void SelectEffectPage(uint8_t slot);
void CheckEditParamOnMain();


#endif