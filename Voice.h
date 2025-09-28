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

using namespace daisy;
using namespace daisysp;

extern std::array<Osc, OSC_NUM> osc;
extern Adsr adsrMain;
extern MoogLadder flt;
extern Oscillator lfo;
extern float samplerate;

void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
void HandlePitchBend(int16_t pitch_bend); 
void SynthInit(float samplerate, int blocksize);
void VoiceProcess(float &sig);
void ModSourcesProcess();

#endif