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

void ProcessEffects(FXSlot &slot, float in, float &outL, float &outR)
{
    if (!slot.isActive)
    {
        outL = in;
        outR = in;
        return;
    }

    if (&slot == &effectSlot[1])
    {
        ProcessEffectsReverb(in, in, outL, outR);
        outR = outR  + (in * effectSlot[1].dryWet); 
        outL = outL + (in * effectSlot[1].dryWet);
        return;
    }
    if (&slot == &effectSlot[0])
    {
        switch (slot.selectedEffect)
        {
        case EFFECT_OVERDRIVE:
            fx.drive.SetDrive(paramManager.GetNormalised(P::EFFECT_OVERDRIVE_DRIVE));
            outL = fx.drive.Process(in);
            outR = outL;
            break;
        case EFFECT_CHORUS:
            fx.chorus.SetLfoFreq(paramManager.GetValue(P::EFFECT_CHORUS_FREQ));
            fx.chorus.SetLfoDepth(paramManager.GetNormalised(P::EFFECT_CHORUS_DEPTH));
            fx.chorus.SetFeedback(paramManager.GetNormalised(P::EFFECT_CHORUS_FBK));
            fx.chorus.SetDelay(paramManager.GetNormalised(P::EFFECT_CHORUS_DELAY));
            fx.chorus.Process(in);
            outL = fx.chorus.GetLeft();
            outR = fx.chorus.GetRight();
            break;
        case EFFECT_COMPRESSOR:
            fx.compressor.SetAttack(paramManager.GetValue(P::EFFECT_COMPRESSOR_ATTACK));
            fx.compressor.SetRelease(paramManager.GetValue(P::EFFECT_COMPRESSOR_RELEASE));
            fx.compressor.SetThreshold(paramManager.GetNormalised(P::EFFECT_COMPRESSOR_THRESHOLD));
            fx.compressor.SetRatio(paramManager.GetNormalised(P::EFFECT_COMPRESSOR_RATIO));
            fx.compressor.SetMakeup(paramManager.GetNormalised(P::EFFECT_COMPRESSOR_MAKEUP));
            outL = fx.compressor.Process(in);
            outR = outL;
            break;
        case EFFECT_NONE:
            outL = in;
            outR = in;
            break;
        default:
            break;
        }
        outR = outR  * (1 - effectSlot[0].dryWet) + (in * effectSlot[0].dryWet); 
        outL = outL * (1 - effectSlot[0].dryWet) + (in * effectSlot[0].dryWet);
    }
}
