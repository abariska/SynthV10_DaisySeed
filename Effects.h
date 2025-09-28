#ifndef EFFECTS_H
#define EFFECTS_H

// TODO implement logic of selesting effects

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"      // Для Overdrive, Chorus, Compressor
#include "daisysp-lgpl.h" // Для Overdrive, Chorus, Compressor
#include "reverb.h"
    
using namespace daisysp;

// Enumeration of effect types
enum EffectName
{
    EFFECT_NONE,
    EFFECT_OVERDRIVE,
    EFFECT_CHORUS,
    EFFECT_COMPRESSOR,
    EFFECT_REVERB,
    EFFECT_COUNT,
};

extern float driveGainCompensation;
extern const char *effectLabels[];

struct FXParam
{
    Overdrive drive;
    Chorus chorus;
    Rev reverb;
    Compressor compressor;
};
extern FXParam fx;

struct FXSlot
{
    EffectName selectedEffect;
    const char *label;
    bool need_update;
    bool isActive;
};
extern FXSlot effectSlot[2];

void EffectsInit(float samplerate);
void ProcessEffects(uint8_t slot, float inL, float inR, float &outL, float &outR);

#endif // EFFECTS_H