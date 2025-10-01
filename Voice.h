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

#define SEMITONE_RATIO 1.0594630943592953f
#define CENT_RATIO 1.0005777895065549f
#define PITCH_TABLE_SIZE 73
#define PITCH_CENTER_INDEX 36
#define DETUNE_TABLE_SIZE 201
#define DETUNE_CENTER_INDEX 100
#define PITCH_BEND_TABLE_SIZE 401
#define PITCH_BEND_CENTER_INDEX 200
#define DTCM __attribute__((section(".dtcm_bss")))

using namespace daisy;
using namespace daisysp;

extern std::array<Osc, OSC_NUM> osc;
extern Adsr adsrMain;
extern MoogLadder flt;
extern Oscillator lfo;
extern float samplerate;

extern float midiNoteToFreqTable[128];
extern float pitchTable[PITCH_TABLE_SIZE];
extern float detuneTable[DETUNE_TABLE_SIZE];
extern float pitchBendTable[PITCH_BEND_TABLE_SIZE];

void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
void HandlePitchBend(int16_t pitch_bend); 
void SynthInit(float samplerate, int blocksize);
void VoiceProcess(float &sig);
void ModSourcesProcess();
void InitPitchTables();
// void midiNoteToFreq(int note);
// float GetPitchTableValue(int index);
// float GetDetuneTableValue(int index);
// float GetPitchBendTableValue(int index);

#endif