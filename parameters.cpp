#include "parameters.h"
#include "per/qspi.h"
#include "sys/dma.h"
#include "voice.h"

ParamSlot paramSlots[NUM_PARAM_BLOCKS];

using namespace daisy;
extern DaisySeed hw;
extern bool page_need_update; 

static const size_t PAGE_SIZE = 1024;           // округлюємо до 512 байт
static const uint32_t FLASH_BASE_ADDR = 0x1000; // Починаємо пресети з 4KB
static const uint32_t FLASH_BLOCK_4KB = 0x1000;

Preset currentPreset;
AudioParamsDirty dirty;
const float default_preset_array[(static_cast<int>(ParamUnitName::COUNT_PARAMS))] = {0.0f,
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f, //Osc1
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, //Osc2
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, //Osc3
                                                                               0.0f, 0.1f, 0.0f, 0.01f, 0.01f, 0.1f, 0.5f, 0.01f, //Filter ADSR
                                                                               0.0f, 0.01f, 1.0f, 0.0f,0.0f, 0.01f, 0.1f, 0.5f, 0.01f, //Mod LFO ADSR
                                                                               0.0f, //Drive
                                                                               0.01f, 0.5f, 0.1f, 0.1f, //Chorus
                                                                               0.01f, 0.01f, 0.5f, 2.0f, 0.5f, //Compressor
                                                                               0.5f, 0.5f, 0.1f, 0.1f, //Flanger
                                                                               0.5f, 0.5f, //Autowah
                                                                               0.5f, 1.0f, //Reverb
                                                                               0.5f, 0.0f, 0.5f, 0.0f, //FX slots
                                                                               0.0f, 0.0f, 0.0f, 0.5f, 0.8f}; //Global

Preset GetDefaultPreset(int8_t presetNumber)
{
    Preset preset = {};
    preset.number = presetNumber;
    preset.type = PresetType::DEFAULT;
    memcpy(preset.array, default_preset_array, sizeof(default_preset_array));

    for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
    {
        preset.mainSlots[i] = MainSlot(); // Викликає конструктор за замовчуванням
    }
    preset.mainSlots[0].target_param = P::FILTER_CUTOFF;
    preset.mainSlots[1].target_param = P::FILTER_RESONANCE;
    preset.mainSlots[2].target_param = P::ADSR_ATTACK;
    preset.mainSlots[3].target_param = P::ADSR_DECAY;

    for (size_t i = 0; i < NUM_FX_SLOTS; i++)
    {
        preset.effectSlots[i] = FXSlot(); // Викликає конструктор за замовчуванням
    }
    preset.effectSlots[0].selectedEffect = EFFECT_NONE;
    preset.effectSlots[1].selectedEffect = EFFECT_NONE;

    for (size_t i = 0; i < MOD_MATRIX_NUM; i++)
    {
        preset.modMtx[i] = ModMatrix(); // Викликає конструктор за замовчуванням
    }

    return preset;
}

void ReadPreset(uint8_t preset_num, Preset &prst);
void SavePreset(uint8_t preset_num, const Preset &prst);
void ResetPreset(int presetNumber);

void InitQSPI()
{
    // Зчитуємо init_flag
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000), PAGE_SIZE);
    uint32_t init_flag = *((uint32_t *)(0x90000000));

    if (init_flag != 0xDEADBEEF)
    {
        hw.qspi.Erase(0, FLASH_BLOCK_4KB);

        uint8_t page[PAGE_SIZE];
        memset(page, 0xFF, sizeof(page));
        uint32_t marker = 0xDEADBEEF;
        memcpy(page, &marker, sizeof(marker));

        hw.qspi.Write(0, sizeof(page), page);

        System::Delay(10);

        dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000), PAGE_SIZE);
        hw.qspi.Erase(FLASH_BASE_ADDR, PRESET_NUM * FLASH_BLOCK_4KB);

        for (size_t i = 0; i < PRESET_NUM; i++)
        {
            uint32_t addr = FLASH_BASE_ADDR + i * FLASH_BLOCK_4KB;
            Preset preset = GetDefaultPreset(i);
            uint8_t page[PAGE_SIZE];
            memset(page, 0, sizeof(page));
            memcpy(page, &preset, sizeof(preset));

            hw.qspi.Write(addr, sizeof(page), page);
            dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
        }
    }
}

// Continuous
SynthParameter::SynthParameter(float min_value, float max_value, const char *full_label, const char *short_label,
                               uint8_t index, float *array, bool is_per_voice, Curve defaultCurve, ParamUnit param_unit, ParamType paramType, UseInMain useInMain, UseInMod useInMod)
    : full_label(full_label),
      short_label(short_label),
      param_index(index),
      param_array(array),
      min(min_value),
      max(max_value),
      curve(defaultCurve),
      unit(param_unit),
      useInMain(useInMain),
      useInMod(useInMod),
      type(paramType),
      is_mod_per_voice(is_per_voice)
{
}

// Discrete
SynthParameter::SynthParameter(int min_vals, int max_vals, const char *full_label, const char *short_label,
                               uint8_t index, float *array, bool is_per_voice, Curve defaultCurve, ParamUnit param_unit, ParamType paramType, UseInMain useInMain, UseInMod useInMod)
    : full_label(full_label),
      short_label(short_label),
      param_index(index),
      param_array(array),
      min(min_vals),
      max(max_vals),
      curve(defaultCurve),
      unit(param_unit),
      useInMain(useInMain),
      useInMod(useInMod),
      type(paramType),
      is_mod_per_voice(is_per_voice)
{
}

void SynthParameter::SetFromCurrentPreset()
{
    float n = currentPreset.array[param_index]; // Read from preset
    SetNormalized(n);                           // Set normalized value
}

float SynthParameter::SetNormalized(float n)
{
    n = clamp(n, 0.0f, 1.0f);
    norm_value = n;
    param_array[param_index] = norm_value;
    if (type == ParamType::DISCRETE)
    {
        if (unit == ParamUnit::BOOL)
        {
            physical_value = norm_value > 0.5f ? 1.0f : 0.0f;
        }
        else
        {
            int num_values = static_cast<int>(max - min) + 1;
            float exact_value = min + norm_value * (num_values - 1);
            physical_value = roundf(exact_value);
            norm_value = (physical_value - min) / (num_values - 1);
            param_array[param_index] = norm_value;
        }
    }
    else
    {
        physical_value = min + norm_value * (max - min); // min..max
    }
    return physical_value;
}

float SynthParameter::SetPhysicalValue(float v)
{
    physical_value = clamp(v, min, max);
    if (type == ParamType::DISCRETE)
    {
        if (unit == ParamUnit::BOOL)
        {
            physical_value = v > 0.5f ? 1.0f : 0.0f;
            norm_value = physical_value > 0.5f ? 1.0f : 0.0f;
        }
        else
        {
        physical_value = roundf(physical_value);
        int num_values = static_cast<int>(max - min) + 1;
            norm_value = (physical_value - min) / (num_values - 1);
            param_array[param_index] = norm_value;
        }
    }
    else
    {
        norm_value = (physical_value - min) / (max - min);
    }
    param_array[param_index] = norm_value;
    return norm_value;
}

float SynthParameter::AdjustByIncrement(int inc)
{
    if (type == ParamType::DISCRETE )
    {
        if (unit == ParamUnit::BOOL)
        {
            float newValue = static_cast<float>(norm_value);
            newValue += inc;
            SetNormalized(newValue);
            return static_cast<float>(newValue);
        }
        else
        {
            int current_int = GetInt();
            int new_int = current_int + inc;
            return SetPhysicalValue(static_cast<float>(new_int));
        }
    }

    switch (unit)
    {
    case ParamUnit::HZ:
    case ParamUnit::FREQ:
    {
        float newValue = 0.0f;
        if (physical_value >= 1.0f)
        {
            float ratio = powf(2.0f, inc / 12.0f);
            newValue = physical_value * ratio;
        }
        else
        {
            if (physical_value >= 0.1f)
            {
                float ratio = powf(2.0f, inc / 6.0f);
                newValue = physical_value * ratio;
            }
            else
            {
                float ratio = powf(2.0f, inc / 2.0f);
                newValue = physical_value * ratio;
            }
        }
        return SetPhysicalValue(newValue);
    }
    case ParamUnit::SECONDS:
    {
        float newValue = 0.0f;
        if (physical_value >= 0.1f)
        {
            float ratio = powf(2.0f, inc / 12.0f);
            newValue = physical_value * ratio;
        }
        else
        {
            float ratio = powf(2.0f, inc / 6.0f);
            newValue = physical_value * ratio;
        }
        return SetPhysicalValue(newValue);
    }
    default:
    {
        float newValue = physical_value + inc;
        return SetPhysicalValue(newValue);
    }
    }
}

float SynthParameter::GetValue() const
{
    float value = 0;
    float m = 0.0f;
    float mod = is_mod_per_voice ? 0.0f : modifier_value; 

    switch (unit)
    {
    case ParamUnit::FREQ:
        value = physical_value;
        return value + (value * mod);
        break;
    case ParamUnit::HZ:
    case ParamUnit::SECONDS:
        value = physical_value;
        m = max;
        return value + ((m - value) * mod);
        break;
    
    case ParamUnit::SEMITONES:
    case ParamUnit::CENTS:
    case ParamUnit::PICTURE:
    case ParamUnit::TEXT:
    case ParamUnit::UNITLESS:
        return GetInt();
        break;
    case ParamUnit::PERCENT:
        value = norm_value;
        m = 1.0f;
        return value + ((m - value) * mod);
        break;
    case ParamUnit::BOOL:
        return static_cast<float>(norm_value > 0.5f ? 1.0f : 0.0f);
        break;
    default:
        return 0.0f;
        break;
    }
}

int SynthParameter::GetInt() const
{
    if (type == ParamType::DISCRETE)
    {
        int range = static_cast<int>(max - min);
        int offset = static_cast<int>(roundf(norm_value * range));
        return static_cast<int>(min) + offset;
    }
    return static_cast<int>(physical_value);
}

void SynthParameter::SetBool(bool value)
{
    float val = value ? 1.0f : 0.0f;
    SetNormalized(val);
}

//--------------------------------
//--------------------------------

void InitSynthParams()
{

    ReadPreset(0, currentPreset);

    paramManager.Init();
    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetFromCurrentPreset();
    }
}
/** --- SavePreset --- */
void SavePreset(uint8_t preset_num, const Preset &prst)
{
    Preset p = prst;
    p.type = PresetType::CUSTOM;
    p.number = prst.number;
    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        p.array[i] = paramManager.GetParam(static_cast<ParamUnitName>(i)).norm_value;
    }
    for (size_t j = 0; j < MOD_MATRIX_NUM; j++)
    {
        p.modMtx[j] = prst.modMtx[j];
    }
    uint32_t addr = FLASH_BASE_ADDR + preset_num * FLASH_BLOCK_4KB;

    hw.qspi.Erase(addr, addr + FLASH_BLOCK_4KB);

    uint8_t page[PAGE_SIZE];
    memset(page, 0, sizeof(page));
    *reinterpret_cast<Preset *>(page) = p;

    hw.qspi.Write(addr, sizeof(page), page);
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    System::Delay(10);
}

/** --- ReadPreset --- */
void ReadPreset(uint8_t preset_num, Preset &prst)
{
    uint32_t addr = FLASH_BASE_ADDR + preset_num * FLASH_BLOCK_4KB;

    uint8_t page[PAGE_SIZE];
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    memcpy(&page, hw.qspi.GetData(addr), sizeof(page));
    prst = *reinterpret_cast<const Preset *>(page);

    System::Delay(10);
}
void ResetPreset(int presetNumber)
{
    Preset preset = GetDefaultPreset(presetNumber);

    uint32_t addr = FLASH_BASE_ADDR + presetNumber * FLASH_BLOCK_4KB;

    hw.qspi.Erase(addr, addr + FLASH_BLOCK_4KB);

    uint8_t page[PAGE_SIZE];
    memset(page, 0, sizeof(page));
    *reinterpret_cast<Preset *>(page) = preset;

    hw.qspi.Write(addr, sizeof(page), page);
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    System::Delay(10);

    ReadPreset(presetNumber, currentPreset);

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetNormalized(currentPreset.array[i]);
    }

    page_need_update = true;
    hw.DelayMs(10);
}

/** --- ApplyPreset --- */
void ApplyPreset(int presetNumber)
{
    ReadPreset(presetNumber, currentPreset);
    for (size_t i = 0; i < VOICE_NUM; i++)
    {
        SynthVoiceReset(i);
    }
    for (size_t i = 0; i < MOD_MATRIX_NUM; i++)
    {
        ModMatrixReset(i);
    }

    ResetModMatrixModulators();

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetNormalized(currentPreset.array[i]);
        paramManager.GetParam(static_cast<ParamUnitName>(i)).modifier_value = 0.0f;
        paramManager.GetParam(static_cast<ParamUnitName>(i)).isDirty = false;
    }
    DirtyFlagsToTrue();
    page_need_update = true;
}

#define ADD_PARAM(param_enum, min_val, max_val, full_label, short_label, is_per_voice, curve, unit, type, useInMain, useInMod) \
    params[static_cast<int>(param_enum)] = SynthParameter(                               \
        min_val, max_val, full_label, short_label, static_cast<int>(param_enum),                           \
        currentPreset.array, is_per_voice, curve, unit, type, useInMain, useInMod)

ParameterManager paramManager;
using P = ParamUnitName;

void ParameterManager::Init()
{
    ADD_PARAM(P::NONE, 0, 0, "-", "-", false, Curve::LINEAR, ParamUnit::UNITLESS, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_WAVEFORM_1, 0, OscWaveforms::WAVE_COUNT - 1, "Wave 1", "Wave", false, Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_1, 1.0f, 10000.0f, "Freq 1", "Freq", true, Curve::EXPONENTIAL, ParamUnit::FREQ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_1, -36, 36, "Pitch 1", "Pitch", true, Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_1, -100, 100, "Tune 1", "Tune", true, Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_1, 0.0f, 100.0f, "Amp 1", "Amp", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_1, -100, 100, "PWM 1", "PWM", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_1, 0, 2, "Enbl Osc1", "Enbl", true, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_WAVEFORM_2, 0, OscWaveforms::WAVE_COUNT - 1, "Wave 2", "Wave", true, Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_2, 1.0f, 10000.0f, "Freq 2", "Freq", true, Curve::EXPONENTIAL, ParamUnit::FREQ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_2, -36, 36, "Pitch 2", "Pitch", true, Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_2, -100, 100, "Tune 2", "Tune", true, Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_2, 0.0f, 100.0f, "Amp 2", "Amp", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_2, -100, 100, "PWM 2", "PWM", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_2, 0, 2, "Enbl Osc2", "Enbl", true, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE); 
    ADD_PARAM(P::OSC_WAVEFORM_3, 0, OscWaveforms::WAVE_COUNT - 1, "Wave 3", "Wave", true, Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_3, 1.0f, 10000.0f, "Freq 3", "Freq", true, Curve::EXPONENTIAL, ParamUnit::FREQ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_3, -36, 36, "Pitch 3", "Pitch", true, Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_3, -100, 100, "Tune 3", "Tune", true, Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_3, 0.0f, 100.0f, "Amp 3", "Amp", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_3, -100, 100, "PWM 3", "PWM", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_3, 0, 2, "Enbl Osc3", "Enbl", true, Curve::LINEAR, ParamUnit::UNITLESS, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::FILTER_MODE, 0, 5, "Mode", "Mode", true,Curve::LINEAR, ParamUnit::TEXT, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::FILTER_CUTOFF, 5.0f, 20000.0f, "Cutoff", "Cutoff", true, Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::FILTER_RESONANCE, 0.0f, 100.0f, "Res", "Res", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::FILTER_DRIVE, 0.0f, 100.0f, "Drive", "Drive", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_ATTACK, 0.005f, 20.0f, "Attack", "Attack", true, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_DECAY, 0.005f, 20.0f, "Decay", "Decay", true, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_SUSTAIN, 0.0f, 100.0f, "Sustain", "Sustain", true, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_RELEASE, 0.005f, 20.0f, "Release", "Release", true, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_LFO_WAVEFORM, 0, OscWaveformsLfo::LFO_WAVE_COUNT - 1, "Wave LFO", "Wave", false, Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::MOD_LFO_FREQ, 0.01f, 100.0f, "Freq Lfo", "Freq", false, Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::MOD_LFO_DEPTH, 0.0f, 100.0f, "Depth Lfo", "Depth", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::MOD_LFO_TRIGGER, 0, 2, "Trig Lfo", "Trigger", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::MOD_LFO_ACTIVE, 0, 2, "ActiveLFO", "Active", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_ATTACK, 0.005f, 20.0f, "Atck Mod", "Attack", false, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_DECAY, 0.005f, 20.0f, "Dec Mod", "Decay", false, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_SUSTAIN, 0.0f, 100.0f, "Sus Mod", "Sustain", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_RELEASE, 0.005f, 20.0f, "Rel Mod", "Release", false, Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::EFFECT_OVERDRIVE_DRIVE, 0.0f, 100.0f, "Drive", "Drive", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_FREQ, 0.1f, 100.0f, "Freq Chrs", "Freq", false, Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_DEPTH, 0.0f, 100.0f, "Dpth Chrs", "Depth", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_FBK, 0.0f, 100.0f, "Fbk Chrs", "Feedback", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_DELAY, 0.0f, 100.0f, "Dly Chrs", "Delay", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_ATTACK, 0.001f, 10.0f, "Atck Com", "Attack", false, Curve::LINEAR, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_RELEASE, 0.001f, 10.0f, "Rel Com", "Release", false, Curve::LINEAR, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_THRESHOLD, -80.0f, 0.0f, "Thrs Com", "Thresh", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_RATIO, 1.0f, 40.0f, "Rat Com", "Ratio", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_MAKEUP, 0.0f, 80.0f, "Mkup Com", "Makeup", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_FLANGER_FEEDBACK, 0.0f, 100.0f, "Fdbk Flg", "Feedbck", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_FLANGER_LFO_DEPTH, 0.0f, 100.0f, "Dpth Flg", "Depth", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_FLANGER_LFO_FREQ, 0.1f, 100.0f, "Freq Flg", "Freq", false, Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_FLANGER_DELAY, 0.0f, 100.0f, "Dly Flg", "Delay", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_AUTOWAH_WAH, 0.0f, 100.0f, "Wah Aut", "Wah", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_AUTOWAH_LEVEL, 0.0f, 100.0f, "Lvl Aut", "Level", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_REVERB_FEEDBACK, 0.0f, 100.0f, "Fdbk Rvb", "Feedbck", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_REVERB_LPFREQ, 10.0f, 20000.0f, "Cut Rvb", "Cutoff", false, Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_1_DRYWET, 0.0f, 100.0f, "FX1", "FX1", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_1_ACTIVE, 0, 2, "Enbl FX1", "Enbl", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::EFFECT_SLOT_2_DRYWET, 0.0f, 100.0f, "FX2", "FX2", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_2_ACTIVE, 0, 2, "Enbl FX2", "Enbl", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_MONO, 0, 2, "Mono", "Mono", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_LEGATO, 0, 2, "Legato", "Legato", false, Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_PORTAMENTO, 0.0f, 100.0f, "Portamento", "Portamento", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_PAN, 0.0f, 100.0f, "Voices Pan", "Pan", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_MASTER_VOLUME, 0.0f, 100.0f, "Master Volume", "Master Volume", false, Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::NONE);
}

void ParameterManager::AdjustByIncrement(ParamUnitName name, int inc) 
{ 
    params[static_cast<int>(name)].AdjustByIncrement(inc); 
    SetAudioDirtyFlag(name);
}

Modulator modulators[static_cast<int>(ModSource::COUNT_MOD_SOURCES)] = {
    {ModSource::NONE, 0.0f, {0.0f}, "-", false},
    {ModSource::LFO, 0.0f, {0.0f}, "LFO", false},
    {ModSource::ADSR, 0.0f, {0.0f}, "ADSR", true},
    {ModSource::MOD_WHEEL, 0.0f, {0.0f}, "ModWheel", false},
    {ModSource::AFTERTOUCH, 0.0f, {0.0f}, "Aftertouch", false},
    {ModSource::VELOCITY, 0.0f, {0.0f}, "Velocity", true},
    {ModSource::SWITCH_PEDAL, 0.0f, {0.0f}, "SW Pedal", false},
};

void ResetModMatrixModulators()
{
    for (size_t i = 0; i < static_cast<int>(ModSource::COUNT_MOD_SOURCES); i++)
    {
        modulators[i].value = 1.0f;
        for (size_t j = 0; j < VOICE_NUM; j++)
        {
            modulators[i].value_per_voice[j] = 1.0f;
        }
    }
}

void SetAudioDirtyFlag(ParamUnitName param) {

    paramManager.GetParam(param).isDirty = true;

    if (param >= P::OSC_WAVEFORM_1 && param <= P::OSC_ACTIVE_3) {
        dirty.oscParams = true;
    }
    if (param >= P::ADSR_ATTACK && param <= P::ADSR_RELEASE) {
        dirty.adsrParams = true;
    }
    else if (param >= P::FILTER_MODE && param <= P::FILTER_DRIVE) {
        dirty.filterParams = true;
    }
    else if (param >= P::MOD_LFO_WAVEFORM && param <= P::MOD_LFO_ACTIVE) {
        dirty.modLfoParams = true;
    }
    else if (param >= P::MOD_ADSR_ATTACK && param <= P::MOD_ADSR_RELEASE) {
        dirty.modAdsrParams = true;
    } 
    else if (param >= P::EFFECT_FLANGER_FEEDBACK && param <= P::EFFECT_FLANGER_DELAY) {
        dirty.flangerParams = true;
    }
    else if (param >= P::EFFECT_CHORUS_FREQ && param <= P::EFFECT_CHORUS_DELAY) {
        dirty.chorusParams = true;
    }
    else if (param >= P::EFFECT_COMPRESSOR_ATTACK && param <= P::EFFECT_COMPRESSOR_MAKEUP) {
        dirty.compressorParams = true;
    }
    else if (param >= P::EFFECT_REVERB_FEEDBACK && param <= P::EFFECT_REVERB_LPFREQ) {
        dirty.reverbParams = true;
    }
    else if (param == P::EFFECT_OVERDRIVE_DRIVE) {
        dirty.driveParams = true;
    }
    else if (param >= P::EFFECT_AUTOWAH_WAH && param <= P::EFFECT_AUTOWAH_LEVEL) {
        dirty.wahParams = true;
    }
    else if (param >= P::GLOBAL_MONO && param <= P::GLOBAL_MASTER_VOLUME) {
        dirty.globalParams = true;
    }
}

void DirtyFlagsToTrue()
{
    dirty.oscParams = true;
    dirty.adsrParams = true;
    dirty.filterParams = true;
    dirty.flangerParams = true;
    dirty.chorusParams = true;
    dirty.compressorParams = true;
    dirty.reverbParams = true;
    dirty.driveParams = true;
    dirty.wahParams = true;
    dirty.modLfoParams = true;
    dirty.modAdsrParams = true;
    dirty.globalParams = true;
}