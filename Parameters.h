#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>

// Required for array structures
#define OSC_NUM 3
#define PARAM_NAME_LENGTH 8

enum class ParamUnitName {
    NONE,
    OSC_WAVEFORM_1,
    OSC_PITCH_1,
    OSC_DETUNE_1,
    OSC_AMP_1,
    OSC_PWM_1,
    OSC_PAN_1,
    OSC_ACTIVE_1,
    OSC_WAVEFORM_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_PWM_2,
    OSC_PAN_2,
    OSC_ACTIVE_2,
    OSC_WAVEFORM_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
    OSC_PWM_3,
    OSC_PAN_3,
    OSC_ACTIVE_3,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    ADSR_RETRIGGER,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    LFO_WAVEFORM,
    LFO_FREQ,
    LFO_DEPTH,
    LFO_ACTIVE,
    EFFECT_OVERDRIVE_DRIVE,
    EFFECT_CHORUS_FREQ,
    EFFECT_CHORUS_DEPTH,
    EFFECT_CHORUS_FBK,
    EFFECT_CHORUS_DELAY,
    EFFECT_COMPRESSOR_ATTACK,
    EFFECT_COMPRESSOR_RELEASE,
    EFFECT_COMPRESSOR_THRESHOLD,
    EFFECT_COMPRESSOR_RATIO,
    EFFECT_COMPRESSOR_MAKEUP,
    EFFECT_REVERB_DRYWET,
    EFFECT_REVERB_FEEDBACK,
    EFFECT_REVERB_LPFREQ,
    GLOBAL_MONO,
    GLOBAL_LEGATO,
    GLOBAL_PORTAMENTO,
    COUNT_PARAMS
};

using P = ParamUnitName;

const P OSC_WAVEFORM[OSC_NUM] = {P::OSC_WAVEFORM_1, P::OSC_WAVEFORM_2, P::OSC_WAVEFORM_3};
const P OSC_PITCH[OSC_NUM] = {P::OSC_PITCH_1, P::OSC_PITCH_2, P::OSC_PITCH_3};
const P OSC_DETUNE[OSC_NUM] = {P::OSC_DETUNE_1, P::OSC_DETUNE_2, P::OSC_DETUNE_3};
const P OSC_AMP[OSC_NUM] = {P::OSC_AMP_1, P::OSC_AMP_2, P::OSC_AMP_3};
const P OSC_PWM[OSC_NUM] = {P::OSC_PWM_1, P::OSC_PWM_2, P::OSC_PWM_3};
const P OSC_PAN[OSC_NUM] = {P::OSC_PAN_1, P::OSC_PAN_2, P::OSC_PAN_3};
const P OSC_ACTIVE[OSC_NUM] = {P::OSC_ACTIVE_1, P::OSC_ACTIVE_2, P::OSC_ACTIVE_3};

extern float parameters_array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];

enum Waves {
    TRI,
    SAW,
    SQR,
    OFF
};

// enum ValueType {
//     REGULAR,
//     X100,
//     WAVEFORM
// };

enum class Curve {
    LINEAR,
    LOGARITHMIC,
    EXPONENTIAL
};

enum class ParamType {
    CONTINUOUS,
    DISCRETE
};

class SynthParameter {
private:
    // Загальні поля
    const char* name_label;
    int param_index;
    float* param_array;
    float norm_value = 0.0f;

    // Для continuous
    float min = 0.0f;
    float max = 1.0f;
    Curve curve = Curve::LINEAR;
    float physical_value = 0.0f;

    // Для discrete
    int max_numbers = 0;
    int discrete_value = 0;

    ParamType type;

public:

    SynthParameter() = default;

    SynthParameter(float init_value, float min_value, float max_value,
        const char* label, uint8_t index, float* array, Curve defaultCurve);

    SynthParameter(int init_value, int max_vals,
        const char* label, uint8_t index, float* array);

    // Універсальні методи
    float SetNormalized(float n) ;

    float SetPhysicalValue(float v);

    float AdjustByEncoder(int inc);

    // Геттери
    float GetFloat() const;
    int GetInt() const;
    float GetNormalised() const;
    bool GetBool() const;
    const char* GetLabel() const;
    ParamType GetType() const;
    float GetMin() const;
    float GetMax() const;
};

class ParameterManager {
    private:
        SynthParameter params[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
        
    public:
        void Init();
        SynthParameter& GetParam(ParamUnitName name) { return params[static_cast<int>(name)]; }
        float GetValue(ParamUnitName name) { return GetParam(name).GetNormalised(); }
        void SetValue(ParamUnitName name, float val) { GetParam(name).SetNormalized(val); }
        const char* GetLabel(ParamUnitName name) { return GetParam(name).GetLabel(); }
    };
    
extern ParameterManager paramManager;


// Functions for initializing parameters
void InitSynthParams();
void InitEffectParams();

// Functions for saving/loading presets
// void SavePreset(uint8_t presetNumber);
// void LoadPreset(uint8_t presetNumber);


#endif // PARAMETERS_H
