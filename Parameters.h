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
#define MOD_MATRIX_NUM 8

template <typename T>
constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

struct Preset;

enum class ParamUnitName
{
    NONE,
    OSC_WAVEFORM_1,
    OSC_FREQ_1,
    OSC_PITCH_1,
    OSC_DETUNE_1,
    OSC_AMP_1,
    OSC_PWM_1,
    OSC_ACTIVE_1,
    OSC_WAVEFORM_2,
    OSC_FREQ_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_PWM_2,
    OSC_ACTIVE_2,
    OSC_WAVEFORM_3,
    OSC_FREQ_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
    OSC_PWM_3,
    OSC_ACTIVE_3,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    MOD_LFO_WAVEFORM,
    MOD_LFO_FREQ,
    MOD_LFO_DEPTH,
    MOD_LFO_ACTIVE,
    MOD_ADSR_ATTACK,
    MOD_ADSR_DECAY,
    MOD_ADSR_SUSTAIN,
    MOD_ADSR_RELEASE,
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
    EFFECT_REVERB_FEEDBACK,
    EFFECT_REVERB_LPFREQ,
    EFFECT_SLOT_1_DRYWET,
    EFFECT_SLOT_1_ACTIVE,
    EFFECT_SLOT_2_DRYWET,
    EFFECT_SLOT_2_ACTIVE,
    GLOBAL_MONO,
    GLOBAL_LEGATO,
    GLOBAL_PORTAMENTO,
    COUNT_PARAMS
};

using P = ParamUnitName;

const P OSC_WAVEFORM[OSC_NUM] = {P::OSC_WAVEFORM_1, P::OSC_WAVEFORM_2, P::OSC_WAVEFORM_3};
const P OSC_FREQ[OSC_NUM] = {P::OSC_FREQ_1, P::OSC_FREQ_2, P::OSC_FREQ_3};
const P OSC_PITCH[OSC_NUM] = {P::OSC_PITCH_1, P::OSC_PITCH_2, P::OSC_PITCH_3};
const P OSC_DETUNE[OSC_NUM] = {P::OSC_DETUNE_1, P::OSC_DETUNE_2, P::OSC_DETUNE_3};
const P OSC_AMP[OSC_NUM] = {P::OSC_AMP_1, P::OSC_AMP_2, P::OSC_AMP_3};
const P OSC_PWM[OSC_NUM] = {P::OSC_PWM_1, P::OSC_PWM_2, P::OSC_PWM_3};
const P OSC_ACTIVE[OSC_NUM] = {P::OSC_ACTIVE_1, P::OSC_ACTIVE_2, P::OSC_ACTIVE_3};

const P EFFECT_SLOT_ACTIVE[2] = {P::EFFECT_SLOT_1_ACTIVE, P::EFFECT_SLOT_2_ACTIVE};
const P EFFECT_SLOT_DRYWET[2] = {P::EFFECT_SLOT_1_DRYWET, P::EFFECT_SLOT_2_DRYWET};

enum Waves
{
    TRI,
    SAW,
    SQR,
    OFF
};

enum class Curve
{
    LINEAR,
    LOGARITHMIC,
    EXPONENTIAL
};

enum class ParamType
{
    CONTINUOUS,
    DISCRETE
};

enum class ParamUnit
{
    HZ,
    SECONDS,
    PERCENT,
    SEMITONES,
    CENTS,
    PICTURE,
    BOOL,
    UNITLESS
};

enum class ModulateableParam
{
    MODULATABLE,
    NOT_MODULATABLE
};

class SynthParameter
{
private:
    // Загальні поля
    const char *name_label;
    int param_index;
    float *param_array;
    float norm_value;
    float min;
    float max;
    Curve curve;
    ParamUnit unit;
    float physical_value;
    float modifier_value;
    ModulateableParam modulateableParam;
    ParamType type;

public:
    SynthParameter() = default;

    SynthParameter(float min_value, float max_value,
                   const char *label, uint8_t index, float *array,
                   Curve defaultCurve,
                   ParamUnit param_unit,
                   ParamType type = ParamType::CONTINUOUS,
                   ModulateableParam modulateableParam = ModulateableParam::NOT_MODULATABLE);

    SynthParameter(int min_vals, int max_vals,
                   const char *label, uint8_t index, float *array,
                   Curve defaultCurve = Curve::LINEAR,
                   ParamUnit param_unit = ParamUnit::UNITLESS,
                   ParamType type = ParamType::DISCRETE,
                   ModulateableParam modulateableParam = ModulateableParam::NOT_MODULATABLE);

    // Універсальні методи
    float SetNormalized(float n);

    float SetPhysicalValue(float v);

    float AdjustByIncrement(int inc);
    void SetNormValue(float value) { norm_value = value; }

    // Геттери
    float GetValue();
    int GetInt() const;
    float GetNormalised() const;
    float GetPhysical() const;
    bool GetBool() const;
    const char *GetLabel() const;
    ParamType GetType() const;
    ParamUnit GetUnit() const;
    void SetBool(bool value);
    void SetFromCurrentPreset();
    void SetFloat(float value) { physical_value = value; }
    Curve GetCurve() const;
    void SetModifier(float value);
    ModulateableParam GetModulateableParam() const { return modulateableParam; }
};

class ParameterManager
{
private:
    SynthParameter params[static_cast<int>(ParamUnitName::COUNT_PARAMS)];

public:
    void Init();
    SynthParameter &GetParam(ParamUnitName name) { return params[static_cast<int>(name)]; }
    float GetNormalised(ParamUnitName name) { return GetParam(name).GetNormalised(); }
    float GetPhysical(ParamUnitName name) { return GetParam(name).GetPhysical(); }
    bool GetBool(ParamUnitName name) { return GetParam(name).GetBool(); }
    const char *GetLabel(ParamUnitName name) { return GetParam(name).GetLabel(); }
    ParamType GetType(ParamUnitName name) { return GetParam(name).GetType(); }
    void AdjustByIncrement(ParamUnitName name, int inc) { GetParam(name).AdjustByIncrement(inc); }
    void SetModifier(ParamUnitName name, float mod_value) { GetParam(name).SetModifier(mod_value); }
    void SetValue(ParamUnitName name, float value) { GetParam(name).SetNormalized(value); }
    void SetBool(ParamUnitName name, bool value) { GetParam(name).SetBool(value); }
    ParamUnit GetUnit(ParamUnitName name) { return GetParam(name).GetUnit(); }
    Curve GetCurve(ParamUnitName name) { return GetParam(name).GetCurve(); }
    void SetFloat(ParamUnitName name, float value) { GetParam(name).SetFloat(value); }
    float GetValue(ParamUnitName name) { return GetParam(name).GetValue(); }
    ModulateableParam GetModulateableParam(ParamUnitName name) { return GetParam(name).GetModulateableParam(); }
};

extern ParameterManager paramManager;

// Functions for initializing parameters
void InitSynthParams();

enum class PresetType : uint8_t
{
    DEFAULT,
    CUSTOM
};

struct Preset
{
    PresetType type;
    uint8_t number;
    char name[PRESET_NAME_LENGTH];
    float array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
};

extern Preset currentPreset;

void ApplyPreset(int presetNumber);
void ReadPreset(uint8_t preset_num, Preset &prst);
void SavePreset(uint8_t preset_num, const Preset &prst);
void InitQSPI();
void ResetPreset(int presetNumber);

enum class ModSource
{
    NONE,
    LFO,
    ADSR,
    MOD_WHEEL,
    COUNT_MOD_SOURCES
};

struct Modulator
{
    ModSource source;
    float value;
    const char *label;
};

extern Modulator modulators[static_cast<int>(ModSource::COUNT_MOD_SOURCES)];

// Mod Matrix
class ModMatrix
{
public:
    ModMatrix()
    {
        modSourceType = ModSource::NONE;
        modTarget = ParamUnitName::NONE;
        modTargetLabel = "-";
        modAmount = 0.0f;
    }

    ModSource modSourceType;
    Modulator modSource;
    ParamUnitName modTarget;
    const char *modTargetLabel;
    float modAmount;

    void SetModSource(ModSource source) { modSourceType = source; }
    void SetModTarget(ParamUnitName target) { modTarget = target; }
    void SetModTargetLabel(const char *label) { modTargetLabel = label; }
    void SetModAmount(float amount) { modAmount = amount; }
    
    // Зчитування значення з глобального масиву modulators
    float GetModValue() const 
    { 
        return modulators[static_cast<int>(modSourceType)].value; 
    }
    
    const char* GetModLabel() const 
    { 
        return modulators[static_cast<int>(modSourceType)].label; 
    }

    void RunMod()
    {
        float modValue = GetModValue();
        modValue = (modValue < 0.0f) ? 0.0f : (modValue > 1.0f) ? 1.0f : modValue;
        modValue = modValue * modAmount;
        paramManager.SetModifier(modTarget, modValue);
    }
};

extern ModMatrix modMatrix[MOD_MATRIX_NUM];

#endif // PARAMETERS_H
