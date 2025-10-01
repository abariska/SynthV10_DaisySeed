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
    fx.reverb.SetFeedback(paramManager.GetValue(P::EFFECT_REVERB_FEEDBACK));
    fx.reverb.SetLpFreq(paramManager.GetValue(P::EFFECT_REVERB_LPFREQ));
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
            fx.drive.SetDrive(paramManager.GetValue(P::EFFECT_OVERDRIVE_DRIVE));
            outL = (fx.drive.Process(inL) + driveGainCompensation) * dryWet + (inL * (1 - dryWet));
            outR = outL;
            break;
        case EFFECT_CHORUS:
            fx.chorus.SetLfoFreq(paramManager.GetValue(P::EFFECT_CHORUS_FREQ));
            fx.chorus.SetLfoDepth(paramManager.GetValue(P::EFFECT_CHORUS_DEPTH));
            fx.chorus.SetFeedback(paramManager.GetValue(P::EFFECT_CHORUS_FBK));
            fx.chorus.SetDelay(paramManager.GetValue(P::EFFECT_CHORUS_DELAY));
            fx.chorus.Process(inL);
            outL = fx.chorus.GetLeft();
            outR = fx.chorus.GetRight();
            outL = inL + (outL * dryWet);
            outR = inR + (outR * dryWet);
            break;
        case EFFECT_COMPRESSOR:
            fx.compressor.SetAttack(paramManager.GetValue(P::EFFECT_COMPRESSOR_ATTACK));
            fx.compressor.SetRelease(paramManager.GetValue(P::EFFECT_COMPRESSOR_RELEASE));
            fx.compressor.SetThreshold(paramManager.GetValue(P::EFFECT_COMPRESSOR_THRESHOLD));
            fx.compressor.SetRatio(paramManager.GetValue(P::EFFECT_COMPRESSOR_RATIO));
            fx.compressor.SetMakeup(paramManager.GetValue(P::EFFECT_COMPRESSOR_MAKEUP));
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
