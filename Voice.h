#ifndef VOICE_H
#define VOICE_H

#include <array>
#include <cstdint>
#include <cmath>
#include "daisy.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"  
#include "Parameters.h"

using namespace daisy;  
using namespace daisysp;

extern std::array<BlOsc, OSC_NUM> osc;
extern Adsr adsrMain;
extern MoogLadder flt;
extern Oscillator lfo;

// Declaration of functions
void InitLfo(float samplerate);
float ProcessLfo();
void HandleNoteOn(uint8_t midi_note, uint8_t midi_vel);
void HandleNoteOff(uint8_t midi_note);
void VoiceInit(float samplerate, int blocksize);
float VoiceProcess();


    
#endif