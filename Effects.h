#ifndef EFFECTS_H
#define EFFECTS_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"      // Для Overdrive, Chorus, Compressor
#include "daisysp-lgpl.h"      // Для Overdrive, Chorus, Compressor
#include "Smallreverb.h"  // Для SmallReverb

using namespace daisy;
using namespace daisysp;

// Forward declaration
struct SynthParams;
extern SynthParams params;
enum EffectType;

struct EffectUnit {
    Overdrive drive;
    Chorus chorus;
    SmallReverb reverb;
    Compressor compressor;
    bool effect_unit_1_active;
    bool effect_unit_2_active;
    bool is_drive_on;
    bool is_chorus_on;
    bool is_compressor_on;
    bool is_reverb_on;

    void Init(float samplerate) {
        drive.Init();
        drive.SetDrive(0.0f);
        is_drive_on = false;

        chorus.Init(samplerate);
        chorus.SetLfoFreq(0.2f);
        chorus.SetPan(0.5f);
        chorus.SetLfoDepth(0.0f);
        chorus.SetFeedback(0.0f);
        chorus.SetDelay(0.0f);
        chorus.SetDelayMs(0.0f);  
        is_chorus_on = false;

        compressor.Init(samplerate);
        compressor.SetAttack(0.01f);
        compressor.SetRelease(0.01f);
        compressor.SetThreshold(0.0f);
        compressor.SetRatio(1.0f);
        compressor.SetMakeup(0.0f);
        is_compressor_on = false;

        reverb.Init(samplerate);
        reverb.SetDryWet(0.5f);
        reverb.SetFeedback(0.97f);
        reverb.SetLpFreq(10000.0f);
        is_reverb_on = false;
        
        effect_unit_1_active = false;
        effect_unit_2_active = false;
    }
    void Process(float inputL, float inputR, float& outputL, float& outputR) {
        float outL = inputL;
        float outR = inputR;
        
        if (is_drive_on) {
            outL = drive.Process(outL);
            outR = drive.Process(outR);
        }
        if (is_chorus_on) {
            outL = chorus.Process(outL);
            outR = chorus.Process(outR);
        }
        if (is_compressor_on) {
            outL = compressor.Process(outL);
            outR = compressor.Process(outR);
        }
        if (is_reverb_on) {
            reverb.Process(outL, outR, &outputL, &outputR);
        } else {
            outputL = outL;
            outputR = outR;
        }
    }
};

struct Effects {
    EffectUnit effect_unit_1;
    EffectUnit effect_unit_2;

    void Init(float samplerate) {
        effect_unit_1.Init(samplerate);
        effect_unit_2.Init(samplerate);
    }

    void Process(float fx_inputL, float fx_inputR, float *fx_outputL, float *fx_outputR) {
        float outL = fx_inputL;
        float outR = fx_inputR;
        if (effect_unit_1.effect_unit_1_active) {
            effect_unit_1.Process(outL, outR, outL, outR);
        }
        if (effect_unit_2.effect_unit_2_active) {
            effect_unit_2.Process(outL, outR, outL, outR);
        }
        *fx_outputL = outL;
        *fx_outputR = outR;
    }
    
    // Метод для оновлення параметрів з структури params
    void UpdateFromParams() {
        // Блок ефектів 1
        effect_unit_1.effect_unit_1_active = params.effectUnits[0].isActive;
        
        // Вимикаємо всі ефекти
        effect_unit_1.is_drive_on = false;
        effect_unit_1.is_chorus_on = false;
        effect_unit_1.is_compressor_on = false;
        effect_unit_1.is_reverb_on = false;
        
        // Вмикаємо потрібний ефект
        switch (params.effectUnits[0].activeEffect) {
            case EFFECT_OVERDRIVE:
                effect_unit_1.is_drive_on = true;
                effect_unit_1.drive.SetDrive(params.effectUnits[0].overdrive.drive);
                break;
            case EFFECT_CHORUS:
                effect_unit_1.is_chorus_on = true;
                effect_unit_1.chorus.SetLfoFreq(params.effectUnits[0].chorus.lfoFreq);
                effect_unit_1.chorus.SetLfoDepth(params.effectUnits[0].chorus.lfoDepth);
                effect_unit_1.chorus.SetDelay(params.effectUnits[0].chorus.delay);
                effect_unit_1.chorus.SetFeedback(params.effectUnits[0].chorus.feedback);
                effect_unit_1.chorus.SetPan(params.effectUnits[0].chorus.pan);
                break;
            case EFFECT_COMPRESSOR:
                effect_unit_1.is_compressor_on = true;
                effect_unit_1.compressor.SetAttack(params.effectUnits[0].compressor.attack);
                effect_unit_1.compressor.SetRelease(params.effectUnits[0].compressor.release);
                effect_unit_1.compressor.SetThreshold(params.effectUnits[0].compressor.threshold);
                effect_unit_1.compressor.SetRatio(params.effectUnits[0].compressor.ratio);
                effect_unit_1.compressor.SetMakeup(params.effectUnits[0].compressor.makeup);
                break;
            case EFFECT_REVERB:
                effect_unit_1.is_reverb_on = true;
                effect_unit_1.reverb.SetFeedback(params.effectUnits[0].reverb.feedback);
                effect_unit_1.reverb.SetLpFreq(params.effectUnits[0].reverb.lpFreq);
                effect_unit_1.reverb.SetDryWet(params.effectUnits[0].reverb.dryWet);
                break;
            case EFFECT_NONE:
                effect_unit_1.effect_unit_1_active = false;
            default:
                // Всі ефекти вимкнені
                break;
        }
        
        // Блок ефектів 2
        effect_unit_2.effect_unit_2_active = params.effectUnits[1].isActive;
        
        // Вимикаємо всі ефекти
        effect_unit_2.is_drive_on = false;
        effect_unit_2.is_chorus_on = false;
        effect_unit_2.is_compressor_on = false;
        effect_unit_2.is_reverb_on = false;
        
        // Вмикаємо потрібний ефект
        switch (params.effectUnits[1].activeEffect) {
            case EFFECT_OVERDRIVE:
                effect_unit_2.is_drive_on = true;
                effect_unit_2.drive.SetDrive(params.effectUnits[1].overdrive.drive);
                break;
            case EFFECT_CHORUS:
                effect_unit_2.is_chorus_on = true;
                effect_unit_2.chorus.SetLfoFreq(params.effectUnits[1].chorus.lfoFreq);
                effect_unit_2.chorus.SetLfoDepth(params.effectUnits[1].chorus.lfoDepth);
                effect_unit_2.chorus.SetDelay(params.effectUnits[1].chorus.delay);
                effect_unit_2.chorus.SetFeedback(params.effectUnits[1].chorus.feedback);
                effect_unit_2.chorus.SetPan(params.effectUnits[1].chorus.pan);
                break;
            case EFFECT_COMPRESSOR:
                effect_unit_2.is_compressor_on = true;
                effect_unit_2.compressor.SetAttack(params.effectUnits[1].compressor.attack);
                effect_unit_2.compressor.SetRelease(params.effectUnits[1].compressor.release);
                effect_unit_2.compressor.SetThreshold(params.effectUnits[1].compressor.threshold);
                effect_unit_2.compressor.SetRatio(params.effectUnits[1].compressor.ratio);
                effect_unit_2.compressor.SetMakeup(params.effectUnits[1].compressor.makeup);
                break;
            case EFFECT_REVERB:
                effect_unit_2.is_reverb_on = true;
                effect_unit_2.reverb.SetFeedback(params.effectUnits[1].reverb.feedback);
                effect_unit_2.reverb.SetLpFreq(params.effectUnits[1].reverb.lpFreq);
                effect_unit_2.reverb.SetDryWet(params.effectUnits[1].reverb.dryWet);
                break;
            case EFFECT_NONE:
                effect_unit_2.effect_unit_2_active = false;
            default:
                // Всі ефекти вимкнені
                break;
        }
    }
};

#endif