#include "effects.h"
#include "parameters.h"
#include "reverb.h"

FXParam fx;

using P = ParamUnitName;

float driveGainCompensation = 0.0f;

const char *effectLabels[] = {
    " - ",
    "Drive",
    "Chorus",
    "Comp",
    "Reverb"};

// Реалізація функцій
void EffectsInit(float samplerate)
{

    fx.drive.Init();
    fx.chorus.Init(samplerate);
    fx.reverb.Init(samplerate);
    fx.compressor.Init(samplerate);
}

void ProcessEffectsReverb(float inL, float inR, float &outL, float &outR)
{
    fx.reverb.Process(inL, inR, &outL, &outR);
}

void ProcessEffects(uint8_t slot, float inL, float inR, float &outL, float &outR)
{
    if (!currentPreset.effectSlots[slot].isActive)
    {
        outL = inL;
        outR = inR;
        return;
    }
    else
    {
        float dryWet = paramManager.GetValue(EFFECT_SLOT_DRYWET[slot]);
        switch (currentPreset.effectSlots[slot].selectedEffect)
        {
        case EFFECT_OVERDRIVE:
            outL = (fx.drive.Process(inL) + driveGainCompensation) * dryWet + (inL * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_CHORUS:            
            outL = fx.chorus.Process(inL);
            outR = fx.chorus.Process(inR);
            outL = inL + (outL * dryWet);
            outR = inR + (outR * dryWet);
            break;
        case EFFECT_COMPRESSOR:
            outL = fx.compressor.Process(inL) * dryWet + (inL * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_REVERB:
            ProcessEffectsReverb(inL, inR, outL, outR);
            outL = inL + (outL * dryWet);
            outR = inR + (outR * dryWet);
            break;
        case EFFECT_NONE:
            outL = inL;
            outR = inR;
            break;
        default:
            break;
        }
    }
}
