#include "parameters.h"
#include "daisysp.h" // Add for using constants

using namespace daisysp;

SynthParams params;

DiscreteParameter::DiscreteParameter(int init_value, int max_vals, uint8_t index, float* array)
    : physical_value(init_value), max_numbers(max_vals), param_index(index), param_array(array) 
    {
        SetNormalized(norm_value);
    }

void DiscreteParameter::SetNormalized(float n){

    if(n > 1.0f) n = 1.0f;
    if(n < 0.0f) n = 0.0f;
    norm_value = n;

    param_array[param_index] = norm_value;
}

void DiscreteParameter::SetPhysical_value(int n){
    physical_value = n;

    if (max_numbers > 1) {
        norm_value = static_cast<float>(physical_value) / (max_numbers - 1);
    } else {
        norm_value = 0.0f;
    }
    param_array[param_index] = norm_value;
}

int DiscreteParameter::AdjustByEncoder(int encoder_inc){
    
    if (encoder_inc != 0) {

        physical_value += encoder_inc;
        if(physical_value > max_numbers - 1) physical_value = max_numbers - 1;
        if(physical_value < 0) physical_value = 0;

        if (max_numbers > 1) {
            float norm_v = 1.0f / (max_numbers - 1) * physical_value;
            SetNormalized(norm_v);
        } else {
            SetNormalized(0.0f);
        }
    }
    return physical_value;
}

float DiscreteParameter::GetNormalised() const {return norm_value;}
int DiscreteParameter::GetInt() const {return physical_value;}


Parameter::Parameter(float init_value, float min_value, float max_value, uint8_t index, 
    float* array, Curve defaultCurve = Curve::LINEAR)
    
    : physical_value(init_value), min(min_value), max(max_value), param_index(index), 
    param_array(array), curve(defaultCurve)
    {
        SetPhysicalValue(physical_value);
    }

    float Parameter::SetNormalized(float n){

        if(n > 1.0f) n = 1.0f;
        if(n < 0.0f) n = 0.0f;
        norm_value = n;

        if (param_index < static_cast<int>(ParamUnitName::NUM_OF_PARAMS)) {
            param_array[param_index] = norm_value;
        } 
    
        switch (curve)
        {
            case Curve::LINEAR: 
                physical_value = min + (norm_value * (max - min));
                break;
            case Curve::EXPONENTIAL:
                physical_value = min + (max - min) * powf(norm_value, 2.0f);
                break;
            case Curve::LOGARITHMIC:
                physical_value = min + (max - min) * powf(norm_value, 0.5f);
                break;
            default:
                break;
        }
        return physical_value;
    }

    float Parameter::SetPhysicalValue(float n) {
        physical_value = n;
    
        switch (curve) {
            case Curve::LINEAR:
                norm_value = (physical_value - min) / (max - min);
                break;
            case Curve::EXPONENTIAL:
                norm_value = sqrtf((physical_value - min) / (max - min));
                break;
            case Curve::LOGARITHMIC:
                norm_value = powf((physical_value - min) / (max - min), 2.0f);
                break;
            default:
                break;
        }
    
        if (norm_value > 1.0f) norm_value = 1.0f;
        if (norm_value < 0.0f) norm_value = 0.0f;
    
        if (param_index < static_cast<int>(ParamUnitName::NUM_OF_PARAMS)) {
            param_array[param_index] = norm_value;
        }
    
        return physical_value;
    }
    
    float Parameter::AdjustByEncoder(int encoder_inc){

        if (encoder_inc != 0) norm_value += encoder_inc * 0.01f;
        return SetNormalized(norm_value);
    }

    float Parameter::GetFloat() const {
        return physical_value;
    }
    
    int Parameter::GetInt() const {
        return static_cast<int>(physical_value);
    }
    
    bool Parameter::GetBool() const {
        return physical_value > 0.5f;
    }



// // Parameters initialization - old version replaced with new one in Parameters.h
// void InitSynthParams() {
//     // Voice template initialization
//     params.osc[0].active = true;
//     params.osc[1].active = false;
//     params.osc[2].active = false;
//     params.osc[0].pan = -1.0f;
//     params.osc[1].pan = 1.0f;
//     params.osc[2].pan = 1.0f;
//     for (size_t o = 0; o < OSC_NUM; o++) {
//         params.osc[o].waveform = 0.0f;
//         params.osc[o].freq = 440.0f;
//         params.osc[o].pw = 0.5f;
//         params.osc[o].amp = 0.5f;
//         params.osc[o].pitch = 0.0f;
//         params.osc[o].detune = 0.0f;
//         // params.osc[o].pan = 0.0f;
//     }
    
//     params.filter.cutoff = 5000.0f;
//     params.filter.resonance = 0.0f;
    
//     params.adsr.attack = 0.01f;
//     params.adsr.decay = 0.1f;
//     params.adsr.sustain = 1.0f;
//     params.adsr.release = 0.5f;
//     params.adsr.retrigger = false;
    
//     // Global LFO initialization
//     params.lfo.freq = 0.5f;
//     params.lfo.depth = 0.0f;
//     params.lfo.waveform = 2.0f;

//     params.global.isMono = true;
//     params.global.isLegato = false;
//     params.global.portamentoTime = 0.0f;
//     params.global.analogAmount = 0.7f;  // 70% аналогового характеру за замовчуванням
    
//     // Effects initialization
//     InitEffectParams();
// }

// void InitEffectParams() {
//     // Initialize parameters for each effect block
//     for (size_t e = 0; e < 2; e++) {
//         // General effect block settings
//         params.overdriveParams.isActive = false;

//         // Overdrive
//         params.overdriveParams.drive = 0.0f;
//         params.overdriveParams.isActive = false;
        
//         // Chorus
//         params.chorusParams.freq = 0.2f;
//         params.chorusParams.depth = 0.0f;
//         params.chorusParams.delay = 0.0f;
//         params.chorusParams.feedback = 0.0f;
//         params.chorusParams.isActive = false;
        
//         // Compressor
//         params.compressorParams.attack = 0.01f;
//         params.compressorParams.release = 0.01f;
//         params.compressorParams.threshold = 0.0f;
//         params.compressorParams.ratio = 1.0f;
//         // params.compressorParams.makeup = 0.0f;
//         params.compressorParams.isActive = false;
        
//         // Reverb
//         params.reverbParams.feedback = 0.0f;
//         params.reverbParams.dryWet = 0.0f;
//         params.reverbParams.isActive = false;
//     }
// }

// Update parameters
// void UpdateParams(Synth& synth, Effects& effects) {
//     // Do nothing to avoid incomplete type issues
//     // Objects are updated directly from parameters in SynthV10.cpp
// }

// // Stubs for preset save/load functions
// void SavePreset(uint8_t presetNumber) {
//     // Code for saving preset will be here
// }

// void LoadPreset(uint8_t presetNumber) {
//     // Code for loading preset will be here
// } 