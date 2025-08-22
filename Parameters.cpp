#include "parameters.h"
#include "daisysp.h" // Add for using constants


SynthParams params;

    SynthParameter::SynthParameter() = default;
    // Continuous
    SynthParameter::SynthParameter(float init_value, float min_value, float max_value,
              const char* label, uint8_t index, float* array,
              Curve defaultCurve = Curve::LINEAR)
        : name_label(label), param_index(index), param_array(array),
          min(min_value), max(max_value), curve(defaultCurve), 
          physical_value(init_value), type(ParamType::CONTINUOUS)
    {
        SetPhysicalValue(init_value);
    }

    // Discrete
    SynthParameter::SynthParameter(int init_value, int max_vals,
              const char* label, uint8_t index, float* array)
        : name_label(label), param_index(index), param_array(array),
          max_numbers(max_vals), discrete_value(init_value), type(ParamType::DISCRETE)
    {
        SetPhysicalValue(init_value);
    }

    // Універсальні методи
    float SynthParameter::SetNormalized(float n) {
        n = std::clamp(n, 0.0f, 1.0f);
        norm_value = n;
        param_array[param_index] = norm_value;

        if(type == ParamType::CONTINUOUS) {
            switch(curve) {
                case Curve::LINEAR:
                    physical_value = min + norm_value * (max - min);
                    break;
                case Curve::EXPONENTIAL:
                    physical_value = min + (max - min) * powf(norm_value, 2.0f);
                    break;
                case Curve::LOGARITHMIC:
                    physical_value = min + (max - min) * powf(norm_value, 0.5f);
                    break;
            }
            return physical_value;
        } else {
            if(max_numbers > 1)
                discrete_value = static_cast<int>(roundf(norm_value * (max_numbers - 1)));
            else
                discrete_value = 0;
            return static_cast<float>(discrete_value);
        }
    }

    float SynthParameter::SetPhysicalValue(float v) {
        if(type == ParamType::CONTINUOUS) {
            physical_value = v;
            float ratio = (physical_value - min) / (max - min);
            switch(curve) {
                case Curve::LINEAR:
                    norm_value = ratio;
                    break;
                case Curve::EXPONENTIAL:
                    norm_value = sqrtf(ratio);
                    break;
                case Curve::LOGARITHMIC:
                    norm_value = powf(ratio, 2.0f);
                    break;
            }
            norm_value = std::clamp(norm_value, 0.0f, 1.0f);
        } else { // DISCRETE
            discrete_value = std::clamp(static_cast<int>(v), 0, max_numbers - 1);
            norm_value = (max_numbers > 1) 
                       ? static_cast<float>(discrete_value) / (max_numbers - 1)
                       : 0.0f;
        }
        param_array[param_index] = norm_value;
        return GetFloat();
    }

    float SynthParameter::AdjustByEncoder(int inc) {
        if(type == ParamType::CONTINUOUS) {
            norm_value += inc * 0.01f;
            return SetNormalized(norm_value);
        } else {
            discrete_value += inc;
            discrete_value = std::clamp(discrete_value, 0, max_numbers - 1);
            return SetPhysicalValue(discrete_value);
        }
    }

    // Геттери
    float SynthParameter::GetFloat() const { return (type == ParamType::CONTINUOUS) ? physical_value : (float)discrete_value; }
    int SynthParameter::GetInt() const { return (type == ParamType::CONTINUOUS) ? static_cast<int>(physical_value) : discrete_value; }
    float SynthParameter::GetNormalised() const { return norm_value; }
    bool SynthParameter::GetBool() const { return GetFloat() > 0.5f; }

SynthParameter parameter[static_cast<int>(ParamUnitName::NUM_OF_PARAMS)];

void InitSynthParams() {
    parameter[0] = SynthParameter(0, 3, "Osc1\nWave", 0, parameters_array);
    parameter[1] = SynthParameter(0, -36, 36, "Osc1\nPitch", 1, parameters_array);
    parameter[2] = SynthParameter(0, 1, "Osc1\nDetune", 2, parameters_array);
    parameter[3] = SynthParameter(0, 1, "Osc1\nAmp", 3, parameters_array);
    parameter[4] = SynthParameter(0, 1, "Osc1\nPan", 4, parameters_array);
    parameter[5] = SynthParameter(0, 1, "Osc1\nActive", 5, parameters_array);
    parameter[6] = SynthParameter(0, 1, "Osc2\nWave", 6, parameters_array);
    parameter[7] = SynthParameter(0, 1, "Osc2\nPitch", 7, parameters_array);
    parameter[8] = SynthParameter(0, 1, "Osc2\nDetune", 8, parameters_array);
    parameter[9] = SynthParameter(0, 1, "Osc2\nAmp", 9, parameters_array);
    parameter[10] = SynthParameter(0, 1, "Osc2\nPan", 10, parameters_array);
    parameter[11] = SynthParameter(0, 1, "Osc2\nActive", 11, parameters_array);
    parameter[12] = SynthParameter(0, 1, "Osc3\nWave", 12, parameters_array);
    parameter[13] = SynthParameter(0, 1, "Osc3\nPitch", 13, parameters_array);
    parameter[14] = SynthParameter(0, 1, "Osc3\nDetune", 14, parameters_array);
    parameter[15] = SynthParameter(0, 1, "Osc3\nAmp", 15, parameters_array);
    parameter[16] = SynthParameter(0, 1, "Osc3\nPan", 16, parameters_array);
    parameter[17] = SynthParameter(0, 1, "Osc3\nActive", 17, parameters_array);
    parameter[18] = SynthParameter(0, 1, "Filter\nCutoff", 18, parameters_array);
    parameter[19] = SynthParameter(0, 1, "Filter\nResonance", 19, parameters_array);
    parameter[20] = SynthParameter(0, 1, "Filter\nActive", 20, parameters_array);
    parameter[21] = SynthParameter(0, 1, "Filter\nType", 21, parameters_array);
    parameter[22] = SynthParameter(0, 1, "ADSR\nAttack", 22, parameters_array);
    parameter[23] = SynthParameter(0, 1, "ADSR\nDecay", 23, parameters_array);
    parameter[24] = SynthParameter(0, 1, "ADSR\nSustain", 24, parameters_array);
    parameter[25] = SynthParameter(0, 1, "ADSR\nRelease", 25, parameters_array);
    parameter[26] = SynthParameter(0, 1, "ADSR\nRetrigger", 26, parameters_array);
    parameter[27] = SynthParameter(0, 1, "LFO\nFreq", 27, parameters_array);
    parameter[28] = SynthParameter(0, 1, "LFO\nDepth", 28, parameters_array);
    parameter[29] = SynthParameter(0, 1, "LFO\nWaveform", 29, parameters_array);

    parameter[30] = SynthParameter(0, 1, "Overdrive\nDrive", 30, parameters_array);
    parameter[31] = SynthParameter(0, 1, "Overdrive\nActive", 31, parameters_array);
    parameter[32] = SynthParameter(0, 1, "Chorus\nFreq", 32, parameters_array);
    parameter[33] = SynthParameter(0, 1, "Chorus\nDepth", 33, parameters_array);
    parameter[34] = SynthParameter(0, 1, "Chorus\nDelay", 34, parameters_array);
    parameter[35] = SynthParameter(0, 1, "Chorus\nFeedback", 35, parameters_array);
    parameter[36] = SynthParameter(0, 1, "Chorus\nActive", 36, parameters_array);
    parameter[37] = SynthParameter(0, 1, "Compressor\nAttack", 37, parameters_array);
    parameter[38] = SynthParameter(0, 1, "Compressor\nRelease", 38, parameters_array);
    parameter[39] = SynthParameter(0, 1, "Compressor\nThreshold", 39, parameters_array);
    parameter[40] = SynthParameter(0, 1, "Compressor\nRatio", 40, parameters_array);
    parameter[41] = SynthParameter(0, 1, "Compressor\nMakeup", 41, parameters_array);
    parameter[42] = SynthParameter(0, 1, "Compressor\nActive", 42, parameters_array);
    parameter[43] = SynthParameter(0, 1, "Reverb\nDryWet", 43, parameters_array);
    parameter[44] = SynthParameter(0, 1, "Reverb\nFeedback", 44, parameters_array);
    parameter[45] = SynthParameter(0, 1, "Reverb\nLPFreq", 45, parameters_array);
    parameter[46] = SynthParameter(0, 1, "Reverb\nActive", 46, parameters_array);
    parameter[47] = SynthParameter(0, 1, "Global\nMono", 47, parameters_array);
    parameter[48] = SynthParameter(0, 1, "Global\nLegato", 48, parameters_array);
    parameter[49] = SynthParameter(0, 1, "Global\nPortamento", 49, parameters_array);
    parameter[50] = SynthParameter(0, 1, "Global\nAnalog", 50, parameters_array);
    parameter[51] = SynthParameter(0, 1, "None", 51, parameters_array);

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