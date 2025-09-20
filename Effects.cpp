#include "effects.h"
#include "parameters.h"
#include "reverb.h"

using P = ParamUnitName;

const char *effectLabels[] = {
    " - ",
    "Drive",
    "Chorus",
    "Comp",
    "Reverb"};

FXParam fx;
FXSlot effectSlot[2];

// Реалізація функцій
void EffectsInit(float samplerate)
{

    fx.drive.Init();
    fx.chorus.Init(samplerate);
    fx.reverb.Init(samplerate);
    fx.compressor.Init(samplerate);

    effectSlot[0].selectedEffect = EFFECT_OVERDRIVE;
    effectSlot[1].selectedEffect = EFFECT_CHORUS;
    effectSlot[0].need_update = false;
    effectSlot[1].need_update = false;
    effectSlot[0].isActive = false;
    effectSlot[1].isActive = false;
    effectSlot[0].label = "";
    effectSlot[1].label = "";
    effectSlot[0].dryWet = 0.5f;
    effectSlot[1].dryWet = 0.5f;
}

void ProcessEffectsReverb(float inL, float inR, float &outL, float &outR)
{
    fx.reverb.SetFeedback(paramManager.GetNormalised(P::EFFECT_REVERB_FEEDBACK));
    fx.reverb.SetLpFreq(paramManager.GetValue(P::EFFECT_REVERB_LPFREQ));
    fx.reverb.Process(inL, inR, &outL, &outR);
}

void ProcessEffects(FXSlot &slot, float inL, float inR, float &outL, float &outR)
{
    if (!slot.isActive)
    {
        outL = inL;
        outR = inR;
        return;
    }
    else
    {
        switch (slot.selectedEffect)
        {
        case EFFECT_OVERDRIVE:
            fx.drive.SetDrive(paramManager.GetNormalised(P::EFFECT_OVERDRIVE_DRIVE));
            outL = fx.drive.Process(inL) * (1 - slot.dryWet) + (inL * slot.dryWet);
            outR = outL;
            break;
        case EFFECT_CHORUS:
            fx.chorus.SetLfoFreq(paramManager.GetValue(P::EFFECT_CHORUS_FREQ));
            fx.chorus.SetLfoDepth(paramManager.GetNormalised(P::EFFECT_CHORUS_DEPTH));
            fx.chorus.SetFeedback(paramManager.GetNormalised(P::EFFECT_CHORUS_FBK));
            fx.chorus.SetDelay(paramManager.GetNormalised(P::EFFECT_CHORUS_DELAY));
            fx.chorus.Process(inL);
            outL = fx.chorus.GetLeft();
            outR = fx.chorus.GetRight();
            outL = inL + (outL * slot.dryWet);
            outR = inR + (outR * slot.dryWet);
            break;
        case EFFECT_COMPRESSOR:
            fx.compressor.SetAttack(paramManager.GetValue(P::EFFECT_COMPRESSOR_ATTACK));
            fx.compressor.SetRelease(paramManager.GetValue(P::EFFECT_COMPRESSOR_RELEASE));
            fx.compressor.SetThreshold(paramManager.GetValue(P::EFFECT_COMPRESSOR_THRESHOLD));
            fx.compressor.SetRatio(paramManager.GetValue(P::EFFECT_COMPRESSOR_RATIO));
            fx.compressor.SetMakeup(paramManager.GetNormalised(P::EFFECT_COMPRESSOR_MAKEUP));
            outL = fx.compressor.Process(inL) * (1 - slot.dryWet) + (inL * slot.dryWet);
            outR = outL;
            break;
        case EFFECT_REVERB:
            ProcessEffectsReverb(inL, inR, outL, outR);
            outL = inL + (outL * slot.dryWet);
            outR = inR + (outR * slot.dryWet);
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
