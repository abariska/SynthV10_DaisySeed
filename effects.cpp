#include "effects.h"
#include "parameters.h"

FXParam fx;

using P = ParamUnitName;

float driveGainCompensation = 0.0f;

void EffectsInit(float samplerate)
{

    fx.drive.Init();
    fx.chorus.Init(samplerate);
    fx.reverb.Init(samplerate);
    fx.compressor.Init(samplerate);
    fx.flanger.Init(samplerate);
    fx.wah.Init(samplerate);
}

inline float ToMono(float L, float R)
{
    return (L + R) * 0.5f;
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
        float mono = ToMono(inL, inR);
        switch (currentPreset.effectSlots[slot].selectedEffect)
        {
        case EFFECT_OVERDRIVE:
            
            outL = (fx.drive.Process(mono) + driveGainCompensation) * dryWet + (mono * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_CHORUS:     
            fx.chorus.Process(mono);
            outL = fx.chorus.GetLeft();
            outR = fx.chorus.GetRight();
            outL = inL + (outL * dryWet);
            outR = inR + (outR * dryWet);
            break;
        case EFFECT_COMPRESSOR:
            outL = fx.compressor.Process(mono) * dryWet + (mono * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_FLANGER:
            outL = fx.flanger.Process(mono) * dryWet + (mono * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_AUTOWAH:
            outL = fx.wah.Process(mono) * dryWet + (mono * (1 - dryWet));
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
