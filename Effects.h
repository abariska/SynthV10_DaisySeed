#ifndef EFFECTS_H
#define EFFECTS_H

// TODO implement logic of selesting effects

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"      // Для Overdrive, Chorus, Compressor
#include "daisysp-lgpl.h"      // Для Overdrive, Chorus, Compressor
#include "Smallreverb.h"  // Для SmallReverb

using namespace daisy;
using namespace daisysp;

// Enumeration of effect types
enum EffectName {
    EFFECT_NONE,
    EFFECT_OVERDRIVE,
    EFFECT_CHORUS,
    EFFECT_COMPRESSOR,
    EFFECT_REVERB,
};

extern const char* effectLabels[];

struct FXSlot {
    EffectName selectedEffect;
    const char* label;
    bool isActive;

    Overdrive drive;
    Chorus chorus;
    SmallReverb reverb;
    Compressor compressor;
};
extern FXSlot effectSlot[2];

void EffectsInit(float samplerate);
void ProcessEffects(FXSlot& slot, float in, float& outL, float& outR);

#endif // EFFECTS_H