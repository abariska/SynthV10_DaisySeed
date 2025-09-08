#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "parameters.h"
#include "daisy_seed.h"
#include "daisysp.h" // Add for using constants
#include "oscillator.h"

// Required for array structures
#define OSC_NUM 3
#define PARAM_NAME_LENGTH 8
#define PRESET_NAME_LENGTH 12
#define PRESET_NUM 10

template<typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

struct Preset;

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
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
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


enum class ParamUnit {
    HZ,
    SECONDS,
    PERCENT,
    SEMITONES,
    CENTS,
    PICTURE,
    BOOL,
    UNITLESS
};

class SynthParameter {
private:
    // Загальні поля
    const char* name_label;
    int param_index;
    float* param_array;
    float norm_value;
    float min;
    float max;
    Curve curve;
    ParamUnit unit;
    float physical_value;

    ParamType type;

public:

    SynthParameter() = default;

    SynthParameter(float min_value, float max_value,
        const char* label, uint8_t index, float* array, 
        Curve defaultCurve, 
        ParamUnit param_unit);

        SynthParameter(int min_vals, int max_vals,
            const char* label, uint8_t index, float* array,  
            Curve defaultCurve = Curve::LINEAR, 
            ParamUnit param_unit = ParamUnit::UNITLESS);

    // Універсальні методи
    float SetNormalized(float n) ;

    float SetPhysicalValue(float v);    

    float AdjustByIncrement(int inc);

    // Геттери
    float GetFloat() const;
    int GetInt() const;
    float GetNormalised() const;
    bool GetBool() const;
    const char* GetLabel() const;
    ParamType GetType() const;
    float GetMin() const;
    float GetMax() const;
    ParamUnit GetUnit() const;
    void SetBool(bool value);
    void ModifyNormalized(float modifier);
    void SetFromCurrentPreset();
    Curve GetCurve() const;
};

class ParameterManager {
    private:
        SynthParameter params[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
        
    public:
        void Init();
        SynthParameter& GetParam(ParamUnitName name) { return params[static_cast<int>(name)]; }
        float GetFloat(ParamUnitName name) { return GetParam(name).GetFloat(); }
        int GetInt(ParamUnitName name) { return GetParam(name).GetInt(); }
        float GetNormalised(ParamUnitName name) { return GetParam(name).GetNormalised(); }
        bool GetBool(ParamUnitName name) { return GetParam(name).GetBool(); }
        const char* GetLabel(ParamUnitName name) { return GetParam(name).GetLabel(); }
        ParamType GetType(ParamUnitName name) { return GetParam(name).GetType(); }
        float GetMin(ParamUnitName name) { return GetParam(name).GetMin(); }
        float GetMax(ParamUnitName name) { return GetParam(name).GetMax(); }
        void AdjustByIncrement(ParamUnitName name, int inc) { GetParam(name).AdjustByIncrement(inc); }
        float GetValue(ParamUnitName name) { return GetParam(name).GetFloat(); }
        void SetValue(ParamUnitName name, float value) { GetParam(name).SetNormalized(value); }
        void SetBool(ParamUnitName name, bool value) { GetParam(name).SetBool(value); }
        ParamUnit GetUnit(ParamUnitName name) { return GetParam(name).GetUnit(); }
        Curve GetCurve(ParamUnitName name) { return GetParam(name).GetCurve(); }
    };
    
extern ParameterManager paramManager;


// Functions for initializing parameters
void InitSynthParams();
void InitEffectParams();

enum class PresetType : uint8_t {
    DEFAULT,
    CUSTOM
};

struct Preset {
    PresetType type; 
    uint8_t number;
    char    name[PRESET_NAME_LENGTH];
    float   array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
};

extern Preset currentPreset;


void ApplyPreset(int presetNumber);
void ReadPreset(uint8_t preset_num, Preset &prst);
void SavePreset(uint8_t preset_num, const Preset &prst);
void InitQSPI();
void ResetPreset(int presetNumber);


#endif // PARAMETERS_H
