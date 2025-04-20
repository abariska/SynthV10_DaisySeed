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

enum EffectName {
    OVERDRIVE,
    CHORUS,
    COMPRESSOR,
    REVERB
};

    Overdrive drive;
    Chorus chorus;
    Compressor compressor;
    SmallReverb reverb;

    const char* labelFX;
    bool isFXActive;

void EffectsInit(float samplerate) {
    drive.Init();
    chorus.Init(samplerate);
    compressor.Init(samplerate);
    reverb.Init(samplerate);
    isFXActive = false;
}

void ProcessFX(float in, float& outL, float& outR) {
    float sigL = in;
    float sigR = in;
    if (!isFXActive) {
        outL = in;
        outR = in;
        return;
    }
    if (params.overdriveParams.isActive) {
        sigL = drive.Process(sigL);
        sigR = drive.Process(sigR);
    }
    if (params.chorusParams.isActive) {
        chorus.Process(sigL);
        sigL += chorus.GetLeft();
        sigR += chorus.GetRight();
    }
    if (params.compressorParams.isActive) {
        sigL = compressor.Process(sigL);
        sigR = compressor.Process(sigR);
    }
    if (params.reverbParams.isActive) {
        reverb.Process(sigL, sigR, &outL, &outR);
    }
    outL = sigL;
    outR = sigR;
}

#endif