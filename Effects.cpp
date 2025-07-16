#include "Effects.h"

const char* effectLabels[] = {
    " - ",
    "Drive",
    "Chorus",
    "Comp",
    "Reverb"
};

FXSlot effectSlot[2];

// Реалізація функцій
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
