#include "parameters.h"
#include "daisysp.h" // Add for using constants

float parameters_array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];

template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

// Continuous
SynthParameter::SynthParameter(float init_value, float min_value, float max_value,
            const char* label, uint8_t index, float* array,
            Curve defaultCurve)
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
    n = clamp(n, 0.0f, 1.0f);
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
            default:
                physical_value = min + norm_value * (max - min);
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
        norm_value = clamp(norm_value, 0.0f, 1.0f);
    } else { // DISCRETE
        discrete_value = clamp(static_cast<int>(v), 0, max_numbers - 1);
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
        discrete_value = clamp(discrete_value, 0, max_numbers - 1);
        return SetPhysicalValue(discrete_value);
    }
}

// Геттери
float SynthParameter::GetFloat() const { return (type == ParamType::CONTINUOUS) ? physical_value : (float)discrete_value; }
int SynthParameter::GetInt() const { return (type == ParamType::CONTINUOUS) ? static_cast<int>(physical_value) : discrete_value; }
float SynthParameter::GetNormalised() const { return norm_value; }
bool SynthParameter::GetBool() const { return GetFloat() > 0.5f; }
const char* SynthParameter::GetLabel() const { return name_label; }
ParamType SynthParameter::GetType() const { return type; }
float SynthParameter::GetMin() const { return (type == ParamType::CONTINUOUS) ? min : 0.0f; }
float SynthParameter::GetMax() const { return (type == ParamType::CONTINUOUS) ? max : (float)max_numbers - 1; }

ParameterManager paramManager;

void ParameterManager::Init() {
    using P = ParamUnitName;
// TODO: Finish with parameters
// TODO: Handle \n in labels
    params[static_cast<int>(P::OSC_WAVEFORM_1)] = SynthParameter(0, 3, "Osc1\nWave", 0, parameters_array);
    params[static_cast<int>(P::OSC_PITCH_1)] = SynthParameter(0.0f, -36.0f, 36.0f, "Osc1\nPitch", 1, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_DETUNE_1)] = SynthParameter(0.0f, 0.0f, 1.0f, "Osc1\nDetune", 2, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_AMP_1)] = SynthParameter(50.0f, 0.0f, 100.0f, "Osc1\nAmp", 3, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PWM_1)] = SynthParameter(0.0f, 0.0f, 100.0f, "Osc1\nPWM", 4, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PAN_1)] = SynthParameter(0.0f, -50.0f, 50.0f, "Osc1\nPan", 5, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_ACTIVE_1)] = SynthParameter(1.0f, 0.0f, 1.0f, "Osc1\nActive", 6, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_WAVEFORM_2)] = SynthParameter(0, 3, "Osc2\nWave", 7, parameters_array);
    params[static_cast<int>(P::OSC_PITCH_2)] = SynthParameter(0.0f, -36.0f, 36.0f, "Osc2\nPitch", 8, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_DETUNE_2)] = SynthParameter(0.0f, 0.0f, 100.0f, "Osc2\nDetune", 9, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_AMP_2)] = SynthParameter(50.0f, 0.0f, 100.0f, "Osc2\nAmp", 10, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PWM_2)] = SynthParameter(0.0f, 0.0f, 100.0f, "Osc2\nPWM", 11, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PAN_2)] = SynthParameter(0.0f, -50.0f, 50.0f, "Osc2\nPan", 12, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_ACTIVE_2)] = SynthParameter(0.0f, 0.0f, 1.0f, "Osc2\nActive", 13, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_WAVEFORM_3)] = SynthParameter(0, 3, "Osc3\nWave", 13, parameters_array);
    params[static_cast<int>(P::OSC_PITCH_3)] = SynthParameter(0.0f, -36.0f, 36.0f, "Osc3\nPitch", 14, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_DETUNE_3)] = SynthParameter(0.0f, 0.0f, 100.0f, "Osc3\nDetune", 15, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_AMP_3)] = SynthParameter(50.0f, 0.0f, 100.0f, "Osc3\nAmp", 16, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PWM_3)] = SynthParameter(0.0f, 0.0f, 100.0f, "Osc3\nPWM", 17, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::OSC_PAN_3)] = SynthParameter(0.0f, -50.0f, 50.0f, "Osc3\nPan", 18, parameters_array, Curve::LINEAR);    
    params[static_cast<int>(P::OSC_ACTIVE_3)] = SynthParameter(0.0f, 0.0f, 1.0f, "Osc3\nActive", 19, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::FILTER_CUTOFF)] = SynthParameter(1000.0f, 10.0f, 15000.0f, "Filter\nCutoff", 20, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::FILTER_RESONANCE)] = SynthParameter(0.0f, 0.0f, 100.0f, "Filter\nResonance", 21, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::ADSR_ATTACK)] = SynthParameter(0.01f, 0.0f, 1000.0f, "ADSR\nAttack", 22, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::ADSR_DECAY)] = SynthParameter(0.1f, 0.0f, 1000.0f, "ADSR\nDecay", 23, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::ADSR_SUSTAIN)] = SynthParameter(1.0f, 0.0f, 100.0f, "ADSR\nSustain", 24, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::ADSR_RELEASE)] = SynthParameter(20.0f, 0.0f, 1000.0f, "ADSR\nRelease", 25, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::ADSR_RETRIGGER)] = SynthParameter(0.0f, 0.0f, 1.0f, "ADSR\nRetrigger", 26, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::LFO_WAVEFORM)] = SynthParameter(0, 3, "LFO\nWaveform", 27, parameters_array); 
    params[static_cast<int>(P::LFO_FREQ)] = SynthParameter(0.0f, 0.0f, 1.0f, "LFO\nFreq", 28, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::LFO_DEPTH)] = SynthParameter(0.0f, 0.0f, 1.0f, "LFO\nDepth", 29, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::LFO_ACTIVE)] = SynthParameter(0.0f, 0.0f, 1.0f, "LFO\nActive", 30, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_CHORUS_DEPTH)] = SynthParameter(0.0f, 0.0f, 1.0f, "Chorus\nDepth", 31, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_CHORUS_FBK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Chorus\nFeedback", 32, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_CHORUS_DELAY)] = SynthParameter(0.0f, 0.0f, 1.0f, "Chorus\nDelay", 33, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_ATTACK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Compressor\nAttack", 34, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RELEASE)] = SynthParameter(0.0f, 0.0f, 1.0f, "Compressor\nRelease", 35, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_THRESHOLD)] = SynthParameter(0.0f, 0.0f, 1.0f, "Compressor\nThreshold", 36, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RATIO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Compressor\nRatio", 37, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_MAKEUP)] = SynthParameter(0.0f, 0.0f, 1.0f, "Compressor\nMakeup", 38, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_REVERB_DRYWET)] = SynthParameter(0.0f, 0.0f, 1.0f, "Reverb\nDryWet", 39, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_REVERB_FEEDBACK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Reverb\nFeedback", 40, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::EFFECT_REVERB_LPFREQ)] = SynthParameter(0.0f, 0.0f, 1.0f, "Reverb\nLPFreq", 41, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::GLOBAL_MONO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Global\nMono", 42, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::GLOBAL_LEGATO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Global\nLegato", 43, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::GLOBAL_PORTAMENTO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Global\nPortamento", 44, parameters_array, Curve::LINEAR);
    params[static_cast<int>(P::NONE)] = SynthParameter(0.0f, 0.0f, 1.0f, "None", 45, parameters_array, Curve::LINEAR);
}

// Використання:
void InitSynthParams() {
    paramManager.Init();
}


// // В main.cpp або де потрібно:
// float osc1_pitch = paramManager.GetValue(ParamUnitName::OSC_PITCH_1);
// paramManager.SetValue(ParamUnitName::OSC_AMP_1, 0.8f);

// // Для енкодера:
// paramManager.GetParam(ParamUnitName::OSC_PITCH_1).AdjustByEncoder(encoder_increment);

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