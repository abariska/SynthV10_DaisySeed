#ifndef EFFECTS_H
#define EFFECTS_H

// TODO implement logic of selesting effects
#include "reverb.h"
#include "daisy.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"

    
using namespace daisysp;

// Enumeration of effect types
enum EffectName
{
    EFFECT_NONE,
    EFFECT_OVERDRIVE,
    EFFECT_FLANGER,
    EFFECT_CHORUS,
    EFFECT_COMPRESSOR,
    EFFECT_AUTOWAH,
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
    Flanger flanger;
    Autowah wah;
};
extern FXParam fx;


void EffectsInit(float samplerate);
void ProcessEffects(uint8_t slot, float inL, float inR, float &outL, float &outR);

#endif // EFFECTS_H
