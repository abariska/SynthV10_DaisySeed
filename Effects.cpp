#include "effects.h"
#include "parameters.h"

using P = ParamUnitName;

// Define the reverb buffer here, once, to be used by the SmallReverb class.
float DSY_SDRAM_BSS delay_buffer_[DSY_SMALLREVERB_MAX_SIZE];

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
}

void ProcessEffects(FXSlot &slot, float in, float &outL, float &outR)
{
    if (!slot.isActive)
    {
        outL = in;
        outR = in;
        return;
    }
    else
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
            outL = fx.chorus.Process(in);
            outR = outL;
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
        case EFFECT_REVERB:
            // fx.reverb.SetDryWet(paramManager.GetNormalised(P::EFFECT_REVERB_DRYWET));
            fx.reverb.SetFeedback(paramManager.GetNormalised(P::EFFECT_REVERB_FEEDBACK));
            fx.reverb.SetLpFreq(paramManager.GetValue(P::EFFECT_REVERB_LPFREQ));
            fx.reverb.Process(in, in, &outL, &outR);
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
