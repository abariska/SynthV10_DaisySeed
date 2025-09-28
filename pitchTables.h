#ifndef PITCH_TABLES_H
#define PITCH_TABLES_H
#include "daisy_seed.h"

#define SEMITONE_RATIO 1.0594630943592953f
#define CENT_RATIO 1.0005777895065549f
#define PITCH_TABLE_SIZE 73
#define PITCH_CENTER_INDEX 36
#define DETUNE_TABLE_SIZE 201
#define DETUNE_CENTER_INDEX 100
#define PITCH_BEND_TABLE_SIZE 401
#define PITCH_BEND_CENTER_INDEX 200

float DSY_SDRAM_BSS midiNoteToFreqTable[128];
float DSY_SDRAM_BSS pitchTable[PITCH_TABLE_SIZE];
float DSY_SDRAM_BSS detuneTable[DETUNE_TABLE_SIZE];
float DSY_SDRAM_BSS pitchBendTable[PITCH_BEND_TABLE_SIZE];

inline void InitPitchTables()
{
    for (int i = 0; i < 128; i++)
    {
        midiNoteToFreqTable[i] = 440.0f * powf(2.0f, (i - 69) / 12.0f);
    }

    for (int i = 0; i < PITCH_TABLE_SIZE; i++)
    {
        pitchTable[i] = powf(2.0f, (i - PITCH_CENTER_INDEX) / 12.0f);
    }
    
    for (int i = 0; i < DETUNE_TABLE_SIZE; i++)
    {
        detuneTable[i] = powf(2.0f, (i - DETUNE_CENTER_INDEX) / 1200.0f);
    }

    for (int i = 0; i < PITCH_BEND_TABLE_SIZE; i++)
    {
        pitchBendTable[i] = powf(2.0f, (i - PITCH_BEND_CENTER_INDEX) / 1200.0f);
    }
}

float midiNoteToFreq(int note)
{
    return midiNoteToFreqTable[note];
}

float GetPitchTableValue(int index)
{
    return pitchTable[index + PITCH_CENTER_INDEX];
}

float GetDetuneTableValue(int index)
{
    return detuneTable[index + DETUNE_CENTER_INDEX];
}

float GetPitchBendTableValue(int index)
{
    return pitchBendTable[index + PITCH_BEND_CENTER_INDEX];
}

#endif