#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisy_seed.h"
#include "daisysp.h" // Add for using constants
#include "globals.h"
#include "oscillator.h"
#include "display.h"
#include "effects.h"

enum class ParamUnitName; 

extern float GetFreqModTableValue(int index);
void SetAudioDirtyFlag(ParamUnitName param);
void SynthVoiceReset(uint8_t voice_num);
void DirtyFlagsToTrue();
void ResetModMatrixModulators();
void InitSynthParams();
template <typename T>
constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (v > hi) ? hi
                                    : v;
}

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

constexpr P OSC_WAVEFORM[OSC_NUM] = {P::OSC_WAVEFORM_1, P::OSC_WAVEFORM_2, P::OSC_WAVEFORM_3};
constexpr P OSC_FREQ[OSC_NUM] = {P::OSC_FREQ_1, P::OSC_FREQ_2, P::OSC_FREQ_3};
constexpr P OSC_PITCH[OSC_NUM] = {P::OSC_PITCH_1, P::OSC_PITCH_2, P::OSC_PITCH_3};
constexpr P OSC_DETUNE[OSC_NUM] = {P::OSC_DETUNE_1, P::OSC_DETUNE_2, P::OSC_DETUNE_3};
constexpr P OSC_AMP[OSC_NUM] = {P::OSC_AMP_1, P::OSC_AMP_2, P::OSC_AMP_3};
constexpr P OSC_PWM[OSC_NUM] = {P::OSC_PWM_1, P::OSC_PWM_2, P::OSC_PWM_3};
constexpr P OSC_ACTIVE[OSC_NUM] = {P::OSC_ACTIVE_1, P::OSC_ACTIVE_2, P::OSC_ACTIVE_3};

constexpr P EFFECT_SLOT_ACTIVE[2] = {P::EFFECT_SLOT_1_ACTIVE, P::EFFECT_SLOT_2_ACTIVE};
constexpr P EFFECT_SLOT_DRYWET[2] = {P::EFFECT_SLOT_1_DRYWET, P::EFFECT_SLOT_2_DRYWET};

constexpr P SETTINGS_PARAMS[SETTINGS_BLOCKS_NUM] = {P::GLOBAL_MONO, P::GLOBAL_LEGATO, P::GLOBAL_PORTAMENTO, P::GLOBAL_PAN, P::GLOBAL_MASTER_VOLUME};

// Flags for ParamDescriptor
constexpr uint8_t FLAG_USE_IN_MAIN = 1 << 0;  // 0b00000001 = 1
constexpr uint8_t FLAG_USE_IN_MOD  = 1 << 1;  // 0b00000010 = 2
constexpr uint8_t FLAG_PER_VOICE   = 1 << 2;  // 0b00000100 = 4

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

struct ParamDescriptor {
    float min, max;
    const char *full_label, *short_label;
    ParamUnit unit;
    ParamType type;
    Curve curve;
    uint8_t flags; // is_per_voice, useInMain, useInMod — біти
};

struct ParamValues {
    float normal;
    float physical;
};

struct ParamMod {
    float mod_global;
    float mod_per_voice[VOICE_NUM];
};

static const ParamDescriptor desc[static_cast<int>(ParamUnitName::COUNT_PARAMS)] = {
    {0, 0, "-", "-", ParamUnit::UNITLESS, ParamType::CONTINUOUS, Curve::LINEAR, 0},
    {0, OscWaveforms::WAVE_COUNT - 1, "Wave 1", "Wave", ParamUnit::PICTURE, ParamType::DISCRETE, Curve::LINEAR, 0},
    {1.0f, 10000.0f, "Freq 1", "Freq", ParamUnit::FREQ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MOD},
    {-36, 36, "Pitch 1", "Pitch", ParamUnit::SEMITONES, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {-100, 100, "Tune 1", "Tune", ParamUnit::CENTS, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Amp 1", "Amp", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {-100, 100, "PWM 1", "PWM", ParamUnit::PERCENT, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Enbl Osc1", "Enbl", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {0, OscWaveforms::WAVE_COUNT - 1, "Wave 2", "Wave", ParamUnit::PICTURE, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {1.0f, 10000.0f, "Freq 2", "Freq", ParamUnit::FREQ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MOD},
    {-36, 36, "Pitch 2", "Pitch", ParamUnit::SEMITONES, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {-100, 100, "Tune 2", "Tune", ParamUnit::CENTS, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Amp 2", "Amp", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {-100, 100, "PWM 2", "PWM", ParamUnit::PERCENT, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Enbl Osc2", "Enbl", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {0, OscWaveforms::WAVE_COUNT - 1, "Wave 3", "Wave", ParamUnit::PICTURE, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {1.0f, 10000.0f, "Freq 3", "Freq", ParamUnit::FREQ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MOD},
    {-36, 36, "Pitch 3", "Pitch", ParamUnit::SEMITONES, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {-100, 100, "Tune 3", "Tune", ParamUnit::CENTS, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Amp 3", "Amp", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {-100, 100, "PWM 3", "PWM", ParamUnit::PERCENT, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Enbl Osc3", "Enbl", ParamUnit::UNITLESS, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {0, 5, "Mode", "Mode", ParamUnit::TEXT, ParamType::DISCRETE, Curve::LINEAR, FLAG_PER_VOICE},
    {5.0f, 20000.0f, "Cutoff", "Cutoff", ParamUnit::HZ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Res", "Res", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Drive", "Drive", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.005f, 20.0f, "Attack", "Attack", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.005f, 20.0f, "Decay", "Decay", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Sustain", "Sustain", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0.005f, 20.0f, "Release", "Release", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_PER_VOICE | FLAG_USE_IN_MAIN},
    {0, OscWaveformsLfo::LFO_WAVE_COUNT - 1, "Wave LFO", "Wave", ParamUnit::PICTURE, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0.01f, 100.0f, "Freq Lfo", "Freq", ParamUnit::HZ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Depth Lfo", "Depth", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Trig Lfo", "Trigger", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0, 2, "ActiveLFO", "Active", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0.005f, 20.0f, "Atck Mod", "Attack", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN},
    {0.005f, 20.0f, "Dec Mod", "Decay", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Sus Mod", "Sustain", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN},
    {0.005f, 20.0f, "Rel Mod", "Release", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN},
    {0.0f, 100.0f, "Drive", "Drive", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.1f, 100.0f, "Freq Chrs", "Freq", ParamUnit::HZ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Dpth Chrs", "Depth", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Fbk Chrs", "Feedback", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Dly Chrs", "Delay", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.001f, 10.0f, "Atck Com", "Attack", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.001f, 10.0f, "Rel Com", "Release", ParamUnit::SECONDS, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {-80.0f, 0.0f, "Thrs Com", "Thresh", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {1.0f, 40.0f, "Rat Com", "Ratio", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 80.0f, "Mkup Com", "Makeup", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Fdbk Flg", "Feedbck", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Dpth Flg", "Depth", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.1f, 100.0f, "Freq Flg", "Freq", ParamUnit::HZ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Dly Flg", "Delay", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Wah Aut", "Wah", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Lvl Aut", "Level", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "Fdbk Rvb", "Feedbck", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {10.0f, 20000.0f, "Cut Rvb", "Cutoff", ParamUnit::HZ, ParamType::CONTINUOUS, Curve::EXPONENTIAL, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0.0f, 100.0f, "FX1", "FX1", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Enbl FX1", "Enbl", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0.0f, 100.0f, "FX2", "FX2", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, FLAG_USE_IN_MAIN | FLAG_USE_IN_MOD},
    {0, 2, "Enbl FX2", "Enbl", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0, 2, "Mono", "Mono", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0, 2, "Legato", "Legato", ParamUnit::BOOL, ParamType::DISCRETE, Curve::LINEAR, 0},
    {0.0f, 100.0f, "Portamento", "Portamento", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, 0},
    {0.0f, 100.0f, "Voices Pan", "Pan", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, 0},
    {0.0f, 100.0f, "Master Volume", "Master Volume", ParamUnit::PERCENT, ParamType::CONTINUOUS, Curve::LINEAR, 0},
};

class ParameterManager
{
private:
    ParamValues values[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
    ParamMod mod[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
    bool dirty[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
public:
    void Init();

    float GetNormalised(ParamUnitName name) const { return values[static_cast<int>(name)].normal; } 
    float GetPhysical(ParamUnitName name) const { return values[static_cast<int>(name)].physical; }
    int GetInt(ParamUnitName name) const;
    bool GetBool(ParamUnitName name) const { return static_cast<float>(values[static_cast<int>(name)].normal) > 0.5f; }
    const char *GetFullLabel(ParamUnitName name) const { return desc[static_cast<int>(name)].full_label; }
    const char *GetShortLabel(ParamUnitName name) const { return desc[static_cast<int>(name)].short_label; }
    ParamType GetType(ParamUnitName name) const { return desc[static_cast<int>(name)].type; } 
    void SetModifier(ParamUnitName name, float mod_value) { mod[static_cast<int>(name)].mod_global = mod_value + 1.0f; }
    void SetModifierPerVoice(ParamUnitName name, int voice_index, float mod_value_per_voice) { mod[static_cast<int>(name)].mod_per_voice[voice_index] = mod_value_per_voice + 1.0f; }
    float SetNormalized(ParamUnitName name, float n);
    float SetPhysicalValue(ParamUnitName name, float v);
    void SetValue(ParamUnitName name, float value) { values[static_cast<int>(name)].physical = value; }
    void SetBool(ParamUnitName name, bool value);
    void SetDirty(ParamUnitName name, bool isDirty);
    float AdjustByIncrement(ParamUnitName name, int inc);
    ParamUnit GetUnit(ParamUnitName name) const { return desc[static_cast<int>(name)].unit; }
    float GetValue(ParamUnitName name) const;
    bool GetUseInMain(ParamUnitName name) const { return desc[static_cast<int>(name)].flags & FLAG_USE_IN_MAIN ? true : false; }
    bool GetUseInMod(ParamUnitName name) const { return desc[static_cast<int>(name)].flags & FLAG_USE_IN_MOD ? true : false; }
    bool GetIsPerVoice(ParamUnitName name) const { return desc[static_cast<int>(name)].flags & FLAG_PER_VOICE ? true : false; }
    float GetModifier(ParamUnitName name) const { return mod[static_cast<int>(name)].mod_global; }
    float GetModifierPerVoice(ParamUnitName name, int voice_index) const { return mod[static_cast<int>(name)].mod_per_voice[voice_index]; }
    bool GetDirty(ParamUnitName name) const { return dirty[static_cast<int>(name)]; }
    void SetFromCurrentPreset(ParamUnitName name);
};

extern ParameterManager paramManager;

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
    float value_per_voice[VOICE_NUM];
    const char *label;
    bool is_per_voice;
};

extern Modulator modulators[static_cast<int>(ModSource::COUNT_MOD_SOURCES)];

struct ModMatrix
{
    ModSource modSource;
    ParamUnitName modTarget;
    float modAmount;

    void ResetMods() 
    {
        modSource = ModSource::NONE;
        modTarget = ParamUnitName::NONE;
        modAmount = 0.0f;
    }
};

struct Preset
{
    uint8_t number;
    float values[static_cast<int>(ParamUnitName::COUNT_PARAMS)];
    ModMatrix modMtx[MOD_MATRIX_NUM];
    MainSlot mainSlots[NUM_MAIN_SLOTS];
    FXSlot effectSlots[NUM_FX_SLOTS];
    PresetType type;
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
