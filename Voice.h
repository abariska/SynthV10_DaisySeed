#ifndef VOICE_H
#define VOICE_H

#include <array>
#include <stdint.h>
#include <cmath>
#include "oscillator.h"
#include "parameters.h"
#include "daisy.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"

#define SEMITONE_RATIO 1.0594630943592953f
#define CENT_RATIO 1.0005777895065549f
#define PITCH_TABLE_SIZE 73
#define PITCH_CENTER_INDEX 36
#define DETUNE_TABLE_SIZE 201
#define DETUNE_CENTER_INDEX 100
#define PITCH_BEND_TABLE_SIZE 401
#define PITCH_BEND_CENTER_INDEX 200
#define FREQ_MOD_TABLE_SIZE 101
#define VOICE_NUM 5 

using namespace daisy;
using namespace daisysp;

// extern std::array<Osc, OSC_NUM> osc;
// extern Adsr adsrMain[VOICE_NUM];

extern MoogLadder flt;
extern Osc lfo;
extern float samplerate;

extern float midiNoteToFreqTable[128];
extern float pitchTable[PITCH_TABLE_SIZE];
extern float detuneTable[DETUNE_TABLE_SIZE];
extern float pitchBendTable[PITCH_BEND_TABLE_SIZE];
extern bool isOscSyncNeeded[OSC_NUM * VOICE_NUM];

struct Voice
{
    Osc     osc[OSC_NUM];
    Adsr    adsr;
    Random  rnd[OSC_NUM];
    float   phaseOffset[OSC_NUM];
    float   pitch_correction[OSC_NUM];
    float   detune_correction[OSC_NUM];
    float   final_freq[OSC_NUM];
    
    bool     active     = false;   // голос зайнятий
    bool     gate       = false;   // флаг для ADSR
    int16_t  note       = -1;      // MIDI-номер ноти
    float    freq       = 0.0f;    // поточна частота
    float    vel        = 1.0f;    // 0…1
    std::uint32_t timestamp  = 0;       // мітка часу (System::GetNow())
};

extern Voice voice[VOICE_NUM];

void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
void HandlePitchBend(int16_t pitch_bend); 
void SynthInit(float samplerate, int blocksize);
void VoiceProcess(float &sig);
void ModSourcesProcess();
void InitPitchTables();
void UpdateModSourcesParams();
void UpdateSynthParams();

#endif