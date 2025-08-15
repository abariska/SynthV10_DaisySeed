#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "parameters.h"
#include "sx1509_expander.h"
#include "voice.h"
#include "effects.h"
#include "midi_handler.h"
#include "daisy_core.h"
#include "display.h"
#include "menu.h"


using namespace daisy;
using namespace daisysp;

// Global objects
extern CpuLoadMeter cpu_load;

extern int encoderIncs[4];
extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;
extern float samplerate;

// Function prototypes
void Timer500ms();
void ProcessButtons();
void ProcessEncoders();
void SelectEffectPage(uint8_t slot);
void InitImages();
void DrawIntroPage();
void SetPage(MenuPage newPage);
void UpdateEncoderSwitches();
void UpdateEncodersParams();
void InitParamBlocks();
void CpuUsageDisplay(bool on = true);


#endif