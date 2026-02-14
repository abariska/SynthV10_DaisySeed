#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisy_seed.h"
#include "daisysp.h" // Add for using constants
#include "oscillator.h"
#include "display.h"
#include "effects.h"

// Required for array structures
#define OSC_NUM 3
#define PRESET_NUM 40
#define MOD_MATRIX_NUM 7

enum class ParamUnitName; 

extern float GetPitchTableValue(int index);
extern float GetDetuneTableValue(int index);
extern float GetPitchBendTableValue(int index);
extern float GetFreqModTableValue(int index);
void SetAudioDirtyFlag(ParamUnitName param);
void SynthVoiceReset(uint8_t voice_num);
void ModMatrixReset(uint8_t mod_matrix_num);
void DirtyFlagsToTrue();
void ResetModMatrixModulators();
template <typename T>
constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (v > hi) ? hi
                                    : v;
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
    FILTER_MODE,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    FILTER_DRIVE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    MOD_LFO_WAVEFORM,
    MOD_LFO_FREQ,
    MOD_LFO_DEPTH,
    MOD_LFO_TRIGGER,
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
    EFFECT_FLANGER_FEEDBACK,
    EFFECT_FLANGER_LFO_DEPTH,
    EFFECT_FLANGER_LFO_FREQ,
    EFFECT_FLANGER_DELAY,
    EFFECT_AUTOWAH_WAH,
    EFFECT_AUTOWAH_LEVEL,
    EFFECT_REVERB_FEEDBACK,
    EFFECT_REVERB_LPFREQ,
    EFFECT_SLOT_1_DRYWET,
    EFFECT_SLOT_1_ACTIVE,
    EFFECT_SLOT_2_DRYWET,
    EFFECT_SLOT_2_ACTIVE,
    GLOBAL_MONO,
    GLOBAL_LEGATO,
    GLOBAL_PORTAMENTO,
    GLOBAL_PAN,
    GLOBAL_MASTER_VOLUME,
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

const P SETTINGS_PARAMS[SETTINGS_BLOCKS_NUM] = {P::GLOBAL_MONO, P::GLOBAL_LEGATO, P::GLOBAL_PORTAMENTO, P::GLOBAL_PAN, P::GLOBAL_MASTER_VOLUME};

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
    FREQ,
    SECONDS,
    PERCENT,
    SEMITONES,
    CENTS,
    PICTURE,
    TEXT,
    BOOL,
    UNITLESS
};

enum class UseInMod
{
    NONE,
    USED
};
enum class UseInMain
{
    NONE,
    USED
};

class SynthParameter
{
private:


public:
    const char *full_label = "-";
    const char *short_label = "-";
    int param_index;
    float *param_array;
    float min;
    float max;
    Curve curve;
    ParamUnit unit;
    UseInMain useInMain;
    UseInMod useInMod;
    ParamType type;
    float modifier_value = 0.0f;
    float norm_value = 0.0f;
    float physical_value = 0.0f;
    bool isDirty = false;

    SynthParameter() = default;

    SynthParameter(float min_value, float max_value,
                   const char *full_label, const char *short_label, uint8_t index, float *array,
                   Curve defaultCurve,
                   ParamUnit param_unit,
                   ParamType type = ParamType::CONTINUOUS,
                   UseInMain useInMain = UseInMain::NONE,
                   UseInMod useInMod = UseInMod::NONE);

    SynthParameter(int min_vals, int max_vals,
                   const char *full_label, const char *short_label, uint8_t index, float *array,
                   Curve defaultCurve = Curve::LINEAR,
                   ParamUnit param_unit = ParamUnit::UNITLESS,
                   ParamType type = ParamType::DISCRETE,
                   UseInMain useInMain = UseInMain::NONE,
                   UseInMod useInMod = UseInMod::NONE);

    // Універсальні методи
    float SetNormalized(float n);

    float SetPhysicalValue(float v);

    float AdjustByIncrement(int inc);
    void SetNormValue(float value) { norm_value = value; }

    // Геттери
    float GetValue();
    int GetInt() const;
    float GetModifier(ParamUnitName name) { return modifier_value; }
    void SetBool(bool value);
    void SetFromCurrentPreset();
    void SetModifier(float value);
};

class ParameterManager
{
private:
    SynthParameter params[static_cast<int>(ParamUnitName::COUNT_PARAMS)];

public:
    void Init();
    SynthParameter &GetParam(ParamUnitName name) { return params[static_cast<int>(name)]; }
    float GetNormalised(ParamUnitName name) { return GetParam(name).norm_value; }
    float GetPhysical(ParamUnitName name) { return GetParam(name).physical_value; }
    bool GetBool(ParamUnitName name) { return static_cast<float>(GetParam(name).norm_value) > 0.5f; }
    const char *GetFullLabel(ParamUnitName name) { return GetParam(name).full_label; }
    const char *GetShortLabel(ParamUnitName name) { return GetParam(name).short_label; }
    ParamType GetType(ParamUnitName name) { return GetParam(name).type; } 
    void AdjustByIncrement(ParamUnitName name, int inc);
    void SetModifier(ParamUnitName name, float mod_value) { GetParam(name).SetModifier(mod_value); }
    void SetValue(ParamUnitName name, float value) { GetParam(name).SetPhysicalValue(value); }
    void SetBool(ParamUnitName name, bool value) { GetParam(name).SetBool(value); }
    ParamUnit GetUnit(ParamUnitName name) { return GetParam(name).unit; }
    Curve GetCurve(ParamUnitName name) { return GetParam(name).curve; }
    float GetValue(ParamUnitName name) { return GetParam(name).GetValue(); }
    UseInMain GetUseInMain(ParamUnitName name) { return GetParam(name).useInMain; }
    UseInMod GetUseInMod(ParamUnitName name) { return GetParam(name).useInMod; }
    float GetModifier(ParamUnitName name) { return GetParam(name).modifier_value; }
};

extern ParameterManager paramManager;

// Functions for initializing parameters
void InitSynthParams();

enum class PresetType : uint8_t
{
    DEFAULT,
    CUSTOM
};

struct ParamSlot
{
    ParamUnitName target_param = P::NONE;
    bool need_update = false;
};
extern ParamSlot paramSlots[NUM_PARAM_BLOCKS];
struct MainSlot
{
    ParamUnitName target_param = P::NONE;
    bool need_update = false;
    bool isEditMode = false;
};
struct FXSlot
{
    EffectName selectedEffect = EFFECT_NONE;
    const char *label = "-";
    bool need_update = false;
    bool isActive = false;
};

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
    AFTERTOUCH,
    VELOCITY,
    SWITCH_PEDAL,
    COUNT_MOD_SOURCES
};

struct Modulator
{
    ModSource source;
    float value;
    const char *label;
    bool isVoiceRelated;
};

extern Modulator modulators[static_cast<int>(ModSource::COUNT_MOD_SOURCES)];
// Mod Matrix
class ModMatrix
{
public:
    ModMatrix()
    {
        modSource.source = ModSource::NONE;
        modSource.value = 0.0f;
        modSource.label = "-";
        modSource.isVoiceRelated = false;
        modTarget = ParamUnitName::NONE;
        modAmount = 0.0f;
    }

public:

    void SetModSource(ModSource source) { 
        modSource.source = modulators[static_cast<int>(source)].source;
        if (modulators[static_cast<int>(source)].isVoiceRelated)
        {
            modSource.isVoiceRelated = true;
        }
        else
        {
            modSource.isVoiceRelated = false;
        }
    }
    void SetModTarget(ParamUnitName target) { modTarget = target; }
    void SetModAmount(float amount) { modAmount = amount; }

    float GetModAmount() const { return modAmount; }
    ModSource GetModSource() const{ return modSource.source; }
    float GetModSourceValue() const { return modulators[static_cast<int>(modSource.source)].value; }
    const char *GetModSourceLabel() const { return modulators[static_cast<int>(modSource.source)].label; }
    ParamUnitName GetModTarget() const { return modTarget; }
    const char *GetModTargetLabel() const { return paramManager.GetFullLabel(modTarget); }
    bool IsModSourceVoiceRelated() const { return modSource.isVoiceRelated; }
    void ResetMods() 
    {
        modSource.value = 0.0f;
        modAmount = 0.0f;
    }

    void RunMod()
    {
        float modValue = GetModSourceValue();
        modValue = (modValue < 0.0f) ? 0.0f : (modValue > 1.0f) ? 1.0f
                                                                : modValue;

        modValue = modValue * modAmount;
        if (paramManager.GetUnit(GetModTarget()) == ParamUnit::FREQ)
        {
            float ratio = GetFreqModTableValue(modAmount * 12.0f); 
            modValue = modValue * ratio;
        }
        paramManager.SetModifier(modTarget, modValue);
        SetAudioDirtyFlag(modTarget);
    }
    
private:
    Modulator modSource;
    ParamUnitName modTarget;
    float modAmount;
};

struct Preset
{
    PresetType type;
    uint8_t number;
    float array[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
    ModMatrix modMtx[MOD_MATRIX_NUM];
    MainSlot mainSlots[NUM_MAIN_SLOTS];
    FXSlot effectSlots[NUM_FX_SLOTS];
};

extern Preset currentPreset;

struct AudioParamsDirty {
    bool oscParams = true;      // OSC_PITCH, OSC_DETUNE, OSC_AMP, OSC_PWM, OSC_WAVEFORM
    bool adsrParams = true;      // ADSR_ATTACK, DECAY, SUSTAIN, RELEASE
    bool filterParams = true;    // FILTER_MODE, CUTOFF, RESONANCE, DRIVE
    bool flangerParams = true;   // EFFECT_FLANGER_*
    bool chorusParams = true;    // EFFECT_CHORUS_*
    bool compressorParams = true;// EFFECT_COMPRESSOR_*
    bool reverbParams = true;    // EFFECT_REVERB_*
    bool driveParams = true;     // EFFECT_OVERDRIVE_DRIVE
    bool wahParams = true;       // EFFECT_AUTOWAH_*
    bool modLfoParams = true;     // MOD_LFO_*
    bool modAdsrParams = true;    // MOD_ADSR_*
    bool globalParams = true;    // GLOBAL_MONO, GLOBAL_LEGATO, GLOBAL_PORTAMENTO, GLOBAL_PAN, GLOBAL_MASTER_VOLUME
};

#endif // PARAMETERS_H
