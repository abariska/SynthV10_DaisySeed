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

const char* effectLabels[] = {
    " - ",
    "Drive",
    "Chorus",
    "Comp",
    "Reverb"
};

struct FXSlot {
    EffectName selectedEffect;
    const char* label;
    bool isActive;

    Overdrive drive;
    Chorus chorus;
    SmallReverb reverb;
    Compressor compressor;
}effectSlot[2];

void EffectsInit(float samplerate) {
    for (size_t i = 0; i < 2; i++) {
        effectSlot[i].drive.Init();
        effectSlot[i].chorus.Init(samplerate);
        effectSlot[i].reverb.Init(samplerate);
        effectSlot[i].compressor.Init(samplerate);
    }
    effectSlot[0].selectedEffect = EFFECT_OVERDRIVE;
    effectSlot[1].selectedEffect = EFFECT_CHORUS;
}

void ProcessEffects(FXSlot& slot, float in, float& outL, float& outR) {
    
    if (!slot.isActive) {
        outL = in;
        outR = in;
        return;
    } else {

    switch (slot.selectedEffect) {
        case EFFECT_OVERDRIVE:
            outL = slot.drive.Process(in);
            outR = outL;
            break;
        case EFFECT_CHORUS:
            outL = slot.chorus.Process(in);
            outR = outL;
            break;
        case EFFECT_COMPRESSOR:
            outL = slot.compressor.Process(in);
            outR = outL;
            break;
        case EFFECT_REVERB:
            slot.reverb.Process(in, in, &outL, &outR);
            break;
        case EFFECT_NONE:
            outL = in;
            outR = in;
            break;
        default:
            break;
    }
    }
}

#endif