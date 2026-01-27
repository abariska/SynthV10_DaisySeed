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
#define PANNING_TABLE_SIZE 101
#define VOICE_NUM 5 

#define MAX_NOTE_STACK 10

using namespace daisy;
using namespace daisysp;

extern LadderFilter flt[2];
extern OscLfo lfo;
extern float samplerate;
extern float lfo_value;
extern float midiNoteToFreqTable[128];
extern float pitchTable[PITCH_TABLE_SIZE];
extern float detuneTable[DETUNE_TABLE_SIZE];
extern float pitchBendTable[PITCH_BEND_TABLE_SIZE];
extern bool isOscSyncNeeded[OSC_NUM * VOICE_NUM];

extern float cached_master_volume;

struct Voice
{
    Osc     osc[OSC_NUM];
    Adsr    adsr;
    Random  rnd[OSC_NUM];
    float   phaseOffset[OSC_NUM];
    float   pitch_correction[OSC_NUM];
    float   detune_correction[OSC_NUM];
    float   final_freq[OSC_NUM];
    float   final_amp[OSC_NUM];
    float   final_pw[OSC_NUM];
    
    bool     active     = false;
    bool     gate       = false;
    int16_t  note       = -1;
    float    freq       = 0.0f;
    float    vel        = 1.0f;
    std::uint32_t timestamp  = 0;
};

extern Voice voice[VOICE_NUM];
extern AudioParamsDirty dirty;

void UpdateCachedFreq(uint8_t voice_num, uint8_t osc_num, float freq);

void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
inline void HandlePitchBend(int16_t pitch_bend); 
void SynthInit(float samplerate, int blocksize);
void VoiceProcess(float &out_sigL, float &out_sigR);
inline void VoicePanning(uint8_t voice_num, float &voice_sig, float &out_L, float &out_R);
inline float softClip(float x);
void ModSourcesProcess();
inline void InitPitchTables();
void UpdateModSourcesParams();
void UpdateSynthParams();
inline void PushNote(uint8_t note);
inline uint8_t PopNote(uint8_t note);
extern float panningTable[101][2];
inline void InitPanningTable();
#endif
