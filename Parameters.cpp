#include "parameters.h"
#include "daisysp.h" // Add for using constants
#include "oscillator.h"

float parameters_array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];

template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

// Continuous
SynthParameter::SynthParameter(float init_value, float min_value, float max_value, const char* label, 
    uint8_t index, float* array, Curve defaultCurve, ParamUnit param_unit)
    : name_label(label), 
        param_index(index), 
        param_array(array), 
        min(min_value), 
        max(max_value), 
        curve(defaultCurve), 
        unit(param_unit), 
        type(ParamType::CONTINUOUS) {
    SetPhysicalValue(init_value);
    }

// Discrete
SynthParameter::SynthParameter(int init_value, int min_vals, int max_vals, 
    const char* label, uint8_t index, float* array, Curve defaultCurve, ParamUnit param_unit)
    : name_label(label),
      param_index(index),
      param_array(array),
      min(min_vals),
      max(max_vals),
      curve(defaultCurve),
      unit(param_unit),
      type(ParamType::DISCRETE) {
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
                physical_value = min + (max - min) * powf(norm_value, 3.0f);
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
        if(max > 1)
            physical_value = static_cast<int>(roundf(norm_value * (max - 1)));
        else
            physical_value = 0;
        return static_cast<float>(physical_value);
    }
}

float SynthParameter::SetPhysicalValue(float v) {
    if(type == ParamType::CONTINUOUS) {
        // Просто обрізаємо і зберігаємо фізичне значення
        physical_value = clamp(v, min, max);
        
        // Лінійне перетворення у нормалізоване
        norm_value = (physical_value - min) / (max - min);
        
    } else { // DISCRETE
        physical_value = clamp(static_cast<int>(v), static_cast<int>(min), static_cast<int>(max - 1));
        norm_value = (max > 1) ? static_cast<float>(physical_value) / (max - 1) : 0.0f;
    }
    
    param_array[param_index] = norm_value;
    return GetFloat();
}

float SynthParameter::AdjustByIncrement(int inc) {

    if (type == ParamType::DISCRETE) {
        float newValue = physical_value + inc;
        return SetPhysicalValue(newValue);
    } else {
        // Нормалізуємо поточне значення
        float ratio = (physical_value - min) / (max - min);
        
        // Застосовуємо криву до ratio для розрахунку кроку
        float step_multiplier;
        switch(curve) {
            case Curve::LINEAR:
                step_multiplier = 1.0f;
                break;
            case Curve::EXPONENTIAL:
                step_multiplier = powf(ratio + 0.01f, 0.7f); // Квадратний корінь
                break;
            case Curve::LOGARITHMIC:
                step_multiplier = powf(ratio + 0.01f, 2.0f); // Квадрат
                break;
            default:
                step_multiplier = 1.0f;
        }
        
        // Базовий крок
        float base_step = (max - min) / 100.0f;
        float adaptive_step = base_step * step_multiplier;
        
        float newValue = physical_value + inc * adaptive_step;
        return SetPhysicalValue(newValue);
    }
}

// Геттери
float SynthParameter::GetFloat() const { return static_cast<float>(physical_value); }
int SynthParameter::GetInt() const { return static_cast<int>(physical_value); }
float SynthParameter::GetNormalised() const { return norm_value; }
bool SynthParameter::GetBool() const { return static_cast<float>(physical_value) > 0.5f; } 
const char* SynthParameter::GetLabel() const { return name_label; }
ParamType SynthParameter::GetType() const { return type; }
float SynthParameter::GetMin() const { return static_cast<float>(min); }
float SynthParameter::GetMax() const { return static_cast<float>(max); }
void SynthParameter::SetBool(bool value) { physical_value = value ? 1 : 0; }
ParamUnit SynthParameter::GetUnit() const { return unit; }

ParameterManager paramManager;

void ParameterManager::Init() {
    using P = ParamUnitName;
// TODO: Finish with parameters
// TODO: Handle \n in labels
    params[static_cast<int>(P::OSC_WAVEFORM_1)] = SynthParameter(0, 0, Osc::WAVE_COUNT, "Wav", 0, parameters_array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_1)] = SynthParameter(0.0f, -36.0f, 36.0f, "Sem", 1, parameters_array, Curve::LINEAR, ParamUnit::SEMITONES);
    params[static_cast<int>(P::OSC_DETUNE_1)] = SynthParameter(0.0f, 0.0f, 100.0f, "Dtn", 2, parameters_array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_1)] = SynthParameter(50.0f, 0.0f, 100.0f, "Amp", 3, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_1)] = SynthParameter(0.0f, 0.0f, 100.0f, "PWM", 4, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_1)] = SynthParameter(0.0f, -100.0f, 100.0f, "Pan", 5, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_ACTIVE_1)] = SynthParameter(1, 0, 2, "Actv", 6, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::OSC_WAVEFORM_2)] = SynthParameter(0, 0, Osc::WAVE_COUNT, "Wav", 7, parameters_array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_2)] = SynthParameter(0.0f, -36.0f, 36.0f, "Sem", 8, parameters_array, Curve::LINEAR, ParamUnit::SEMITONES);
    params[static_cast<int>(P::OSC_DETUNE_2)] = SynthParameter(0.0f, 0.0f, 100.0f, "Dtn", 9, parameters_array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_2)] = SynthParameter(50.0f, 0.0f, 100.0f, "Amp", 10, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_2)] = SynthParameter(0.0f, 0.0f, 100.0f, "Pwm", 11, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_2)] = SynthParameter(0.0f, -100.0f, 100.0f, "Pan", 12, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_ACTIVE_2)] = SynthParameter(0, 0, 2, "Actv", 13, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::OSC_WAVEFORM_3)] = SynthParameter(0, 0, Osc::WAVE_COUNT, "Wav", 14, parameters_array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_3)] = SynthParameter(0.0f, -36.0f, 36.0f, "Sem", 15, parameters_array, Curve::LINEAR, ParamUnit::SEMITONES);
    params[static_cast<int>(P::OSC_DETUNE_3)] = SynthParameter(0.0f, 0.0f, 100.0f, "Dtn", 16, parameters_array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_3)] = SynthParameter(50.0f, 0.0f, 100.0f, "Amp", 17, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_3)] = SynthParameter(0.0f, 0.0f, 100.0f, "PWM", 18, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_3)] = SynthParameter(0.0f, -100.0f, 100.0f, "Pan", 19, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);    
    params[static_cast<int>(P::OSC_ACTIVE_3)] = SynthParameter(0, 0, 2, "Actv", 20, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::FILTER_CUTOFF)] = SynthParameter(1000.0f, 10.0f, 15000.0f, "Cut", 21, parameters_array, Curve::EXPONENTIAL, ParamUnit::HZ);
    params[static_cast<int>(P::FILTER_RESONANCE)] = SynthParameter(0.0f, 0.0f, 100.0f, "Res", 22, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::ADSR_ATTACK)] = SynthParameter(0.01f, 0.0f, 10000.0f, "Atk", 23, parameters_array, Curve::EXPONENTIAL, ParamUnit::MS);
    params[static_cast<int>(P::ADSR_DECAY)] = SynthParameter(0.1f, 0.0f, 10000.0f, "Dcy", 24, parameters_array, Curve::EXPONENTIAL, ParamUnit::MS);
    params[static_cast<int>(P::ADSR_SUSTAIN)] = SynthParameter(1.0f, 0.0f, 100.0f, "Sus", 25, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::ADSR_RELEASE)] = SynthParameter(20.0f, 0.0f, 10000.0f, "Rls", 26, parameters_array, Curve::EXPONENTIAL, ParamUnit::MS);
    params[static_cast<int>(P::ADSR_RETRIGGER)] = SynthParameter(0, 0, 2, "Rtr", 27, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::LFO_WAVEFORM)] = SynthParameter(0, 0, Osc::WAVE_COUNT, "Wav", 28, parameters_array, Curve::LINEAR, ParamUnit::PICTURE); 
    params[static_cast<int>(P::LFO_FREQ)] = SynthParameter(0.0f, 0.0f, 100.0f, "Frq", 29, parameters_array, Curve::LINEAR, ParamUnit::HZ);
    params[static_cast<int>(P::LFO_DEPTH)] = SynthParameter(0.0f, 0.0f, 1.0f, "Dpt", 30, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::LFO_ACTIVE)] = SynthParameter(0, 0, 2, "Actv", 31, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::EFFECT_CHORUS_DEPTH)] = SynthParameter(0.0f, 0.0f, 1.0f, "Dpt", 32, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_CHORUS_FBK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Fbk", 33, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_CHORUS_DELAY)] = SynthParameter(0.0f, 0.0f, 1.0f, "Dly", 34, parameters_array, Curve::LINEAR, ParamUnit::MS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_ATTACK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Atk", 35, parameters_array, Curve::LINEAR, ParamUnit::MS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RELEASE)] = SynthParameter(0.0f, 0.0f, 1.0f, "Rls", 36, parameters_array, Curve::LINEAR, ParamUnit::MS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_THRESHOLD)] = SynthParameter(0.0f, 0.0f, 1.0f, "Thr", 37, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RATIO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Rat", 38, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_MAKEUP)] = SynthParameter(0.0f, 0.0f, 1.0f, "Mk", 39, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_DRYWET)] = SynthParameter(0.0f, 0.0f, 1.0f, "DrW", 40, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_FEEDBACK)] = SynthParameter(0.0f, 0.0f, 1.0f, "Fbk", 41, parameters_array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_LPFREQ)] = SynthParameter(0.0f, 0.0f, 1.0f, "LpF", 42, parameters_array, Curve::LINEAR, ParamUnit::HZ);
    params[static_cast<int>(P::GLOBAL_MONO)] = SynthParameter(0, 0, 2, "Mon", 43, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::GLOBAL_LEGATO)] = SynthParameter(0, 0, 2, "Lgt", 44, parameters_array, Curve::LINEAR, ParamUnit::UNITLESS);
    params[static_cast<int>(P::GLOBAL_PORTAMENTO)] = SynthParameter(0.0f, 0.0f, 1.0f, "Prt", 45, parameters_array, Curve::LINEAR, ParamUnit::MS);
    params[static_cast<int>(P::NONE)] = SynthParameter(0, 0, 0, "", 46, parameters_array);
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