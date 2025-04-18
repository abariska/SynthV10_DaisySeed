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

extern SynthParams params;

// LFO is declared as global for all voices
extern BlOsc Lfo;
extern float lfoValue;

const uint8_t NUM_VOICES = 3;

// Declaration of functions
void InitLfo(float samplerate);
float ProcessLfo();
void HandleNoteOn(uint8_t midi_note, uint8_t midi_vel);
void HandleNoteOff(uint8_t midi_note);

struct VoiceUnit
{
    std::array<BlOsc, OSC_NUM> osc;
    Adsr adsrMain;
    MoogLadder flt;

    bool isVoiceActive;
    bool isGated;
    uint8_t note;
    uint8_t velocity;
    uint32_t timestamp;

    void Init(float samplerate, int blocksize);
    void NoteOn(uint8_t midi_note, uint8_t midi_vel, uint32_t time);
    void NoteOff();
    float CalculateFrequency(uint8_t midi_note, int pitch, int detune);
    float CalculateVelocity(uint8_t velocity, uint8_t midi_note);
    float Process();
};


    
#endif