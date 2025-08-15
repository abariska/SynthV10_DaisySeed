#ifndef VOICE_H
#define VOICE_H

#include <array>
#include <cstdint>
#include <cmath>
#include "daisy.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"  
#include "parameters.h"
#include "oscillator.h"
// #include "phase_generator.h"

using namespace daisy;
using namespace daisysp;

// extern PhaseGenerator phaseGenerator;
extern std::array<Osc, OSC_NUM> osc;
extern Adsr adsrMain;
extern MoogLadder flt;
extern Oscillator lfo;


// Declaration of functions
void InitLfo(float samplerate);
float ProcessLfo();
void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
void VoiceInit(float samplerate, int blocksize);
void VoiceProcess(float& sigL, float& sigR);
void VoiceProcessTest(float& sigL, float& sigR);


    
#endif