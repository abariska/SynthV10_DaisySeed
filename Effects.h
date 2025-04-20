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
    isFXActive = true;
}

void ProcessFX(float in, float& outL, float& outR) {
    drive.SetDrive(params.overdriveParams.drive);
    chorus.SetLfoFreq(params.chorusParams.freq);
    chorus.SetLfoDepth(params.chorusParams.depth);
    chorus.SetFeedback(params.chorusParams.feedback);
    chorus.SetPan(params.chorusParams.pan);
    compressor.SetAttack(params.compressorParams.attack);
    compressor.SetRelease(params.compressorParams.release);
    compressor.SetThreshold(params.compressorParams.threshold);
    compressor.SetRatio(params.compressorParams.ratio);
    reverb.SetFeedback(params.reverbParams.feedback);
    reverb.SetLpFreq(params.reverbParams.lpFreq);
    reverb.SetDryWet(params.reverbParams.dryWet);

    float sigL = in;
    float sigR = in;
    if (!isFXActive) {
        outL = in;
        outR = in;
        return;
    }
    if (params.overdriveParams.isActive) {
        sigL += drive.Process(sigL);
        // sigL *= params.overdriveParams.makeup;
        sigR = sigL;
    }
    if (params.compressorParams.isActive) {
        sigL = compressor.Process(sigL);
        sigR = sigL;
    }
    if (params.chorusParams.isActive) {
        chorus.Process(sigL);
        sigL += chorus.GetLeft();
        sigR += chorus.GetRight();
    }
    if (params.reverbParams.isActive) {
        float wetRevL;
        float wetRevR;
        reverb.Process(sigL, sigR, &wetRevL, &wetRevR);
        outL = (1 - params.reverbParams.dryWet) * sigL + params.reverbParams.dryWet * wetRevL;
        outR = (1 - params.reverbParams.dryWet) * sigR + params.reverbParams.dryWet * wetRevR;
    } else {
        outL = sigL;
        outR = sigR;
    }
}

#endif