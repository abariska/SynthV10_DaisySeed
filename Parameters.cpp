#include "parameters.h"
#include "daisy_seed.h"
#include "daisysp.h" // Add for using constants
#include "oscillator.h"
#include "display.h"
#include "log_uart.h"
#include "per/qspi.h"
#include "sys/dma.h"

// #define DTCM __attribute__((section(".dtcm_bss")))
ParamSlot paramSlots[NUM_PARAM_BLOCKS];

using namespace daisy;
extern DaisySeed hw;
extern bool page_need_update;

static const size_t PAGE_SIZE = 1024;           // округлюємо до 512 байт
static const uint32_t FLASH_BASE_ADDR = 0x1000; // Починаємо пресети з 4KB
static const uint32_t FLASH_BLOCK_4KB = 0x1000;

Preset currentPreset;
float default_preset_array[(static_cast<int>(ParamUnitName::COUNT_PARAMS))] = {0.0f,
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.2f, 0.5f, 1.0f, //Osc1
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.2f, 0.5f, 0.0f, //Osc2
                                                                               0.0f, 0.0f, 0.5f, 0.5f, 0.2f, 0.5f, 0.0f, //Osc3
                                                                               0.1f, 0.0f, 0.01f, 0.01f, 1.0f, 0.01f, //Filter ADSR
                                                                               0.0f, 0.01f, 0.1f, 0.0f, 0.01f, 0.01f, 0.0f, 0.01f, //Mod LFO ADSR
                                                                               0.0f, //Drive
                                                                               0.1f, 0.5f, 0.5f, 0.5f, //Chorus
                                                                               0.01f, 0.01f, 0.5f, 2.0f, 0.5f, //Compressor
                                                                               0.5f, 1.0f, //Reverb
                                                                               0.5f, 0.0f, 0.5f, 0.0f, //FX slots
                                                                               1.0f, 0.0f, 0.1f}; //Global

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
    preset.effectSlots[0].selectedEffect = EFFECT_CHORUS;
    preset.effectSlots[1].selectedEffect = EFFECT_REVERB;

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
SynthParameter::SynthParameter(float min_value, float max_value, const char *label,
                               uint8_t index, float *array, Curve defaultCurve, ParamUnit param_unit, ParamType paramType, UseInMain useInMain, UseInMod useInMod)
    : name_label(label),
      param_index(index),
      param_array(array),
      min(min_value),
      max(max_value),
      curve(defaultCurve),
      unit(param_unit),
      useInMain(useInMain),
      useInMod(useInMod),
      type(paramType)
{
}

// Discrete
SynthParameter::SynthParameter(int min_vals, int max_vals, const char *label,
                               uint8_t index, float *array, Curve defaultCurve, ParamUnit param_unit, ParamType paramType, UseInMain useInMain, UseInMod useInMod)
    : name_label(label),
      param_index(index),
      param_array(array),
      min(min_vals),
      max(max_vals),
      curve(defaultCurve),
      unit(param_unit),
      useInMain(useInMain),
      useInMod(useInMod),
      type(paramType)
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
            norm_value = physical_value;
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
            float newValue = static_cast<float>(GetNormalised());
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

void SynthParameter::SetModifier(float value)
{
    modifier_value = value;
}

float SynthParameter::GetValue()
{
    float value = 0;
    float m = 0.0f;
    switch (unit)
    {
    case ParamUnit::HZ:
    case ParamUnit::SECONDS:
        value = physical_value;
        m = max;
        return value + ((m - value) * modifier_value);
        break;
    
    case ParamUnit::SEMITONES:
    case ParamUnit::CENTS:
    case ParamUnit::PICTURE:
    case ParamUnit::UNITLESS:
        return static_cast<float>(GetInt());
        break;
    case ParamUnit::PERCENT:
        value = norm_value;
        m = 1.0f;
        return value + ((m - value) * modifier_value);
        break;
    case ParamUnit::BOOL:
        return static_cast<float>(GetBool());
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

// Getters
float SynthParameter::GetNormalised() const { return norm_value; }
bool SynthParameter::GetBool() const { return static_cast<float>(norm_value) > 0.5f; }
const char *SynthParameter::GetLabel() const { return name_label; }
ParamType SynthParameter::GetType() const { return type; }
ParamUnit SynthParameter::GetUnit() const { return unit; }
Curve SynthParameter::GetCurve() const { return curve; }
float SynthParameter::GetPhysical() const { return physical_value; }

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

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        float v = paramManager.GetParam(static_cast<ParamUnitName>(i)).GetNormalised();
        p.type = PresetType::CUSTOM;
        p.number = prst.number;
        p.array[i] = v;
        for (size_t j = 0; j < MOD_MATRIX_NUM; j++)
        {
            p.modMtx[j] = prst.modMtx[j]; // Викликає конструктор за замовчуванням
        }
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

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetNormalized(currentPreset.array[i]);
    }
    page_need_update = true;
}

#define ADD_PARAM(param_enum, min_val, max_val, label, curve, unit, type, useInMain, useInMod) \
    params[static_cast<int>(param_enum)] = SynthParameter(                               \
        min_val, max_val, label, static_cast<int>(param_enum),                           \
        currentPreset.array, curve, unit, type, useInMain, useInMod)

ParameterManager paramManager;
using P = ParamUnitName;

void ParameterManager::Init()
{
    ADD_PARAM(P::NONE, 0, 0, "-", Curve::LINEAR, ParamUnit::UNITLESS, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_WAVEFORM_1, 0, Osc::WAVE_COUNT - 1, "Wav1", Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_1, 1.0f, 10000.0f, "Frq1", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_1, -36, 36, "Sem1", Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_1, -100, 100, "Dtn1", Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_1, 0.0f, 100.0f, "Amp1", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_1, -100, 100, "Pwm1", Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_1, 0, 2, "Actv1", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_WAVEFORM_2, 0, Osc::WAVE_COUNT - 1, "Wav2", Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_2, 1.0f, 10000.0f, "Frq2", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_2, -36, 36, "Sem2", Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_2, -100, 100, "Dtn2", Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_2, 0.0f, 100.0f, "Amp2", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_2, -100, 100, "Pwm2", Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_2, 0, 2, "Actv2", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE); 
    ADD_PARAM(P::OSC_WAVEFORM_3, 0, Osc::WAVE_COUNT - 1, "Wav3", Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::OSC_FREQ_3, 1.0f, 10000.0f, "Frq3", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::NONE, UseInMod::USED);
    ADD_PARAM(P::OSC_PITCH_3, -36, 36, "Sem3", Curve::LINEAR, ParamUnit::SEMITONES, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_DETUNE_3, -100, 100, "Dtn3", Curve::LINEAR, ParamUnit::CENTS, ParamType::DISCRETE, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::OSC_AMP_3, 0.0f, 100.0f, "Amp3", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_PWM_3, -100, 100, "Pwm3", Curve::LINEAR, ParamUnit::PERCENT, ParamType::DISCRETE, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::OSC_ACTIVE_3, 0, 2, "Actv3", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::FILTER_CUTOFF, 20.0f, 20000.0f, "Cut", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::FILTER_RESONANCE, 0.0f, 100.0f, "Res", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::ADSR_ATTACK, 0.005f, 20.0f, "Atk", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_DECAY, 0.005f, 20.0f, "Dcy", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_SUSTAIN, 0.0f, 100.0f, "Sus", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::ADSR_RELEASE, 0.005f, 20.0f, "Rls", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_LFO_WAVEFORM, 0, Osc::WAVE_COUNT - 1, "Wav", Curve::LINEAR, ParamUnit::PICTURE, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::MOD_LFO_FREQ, 0.01f, 100.0f, "Frq", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::MOD_LFO_DEPTH, 0.0f, 100.0f, "Dpt", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::MOD_LFO_ACTIVE, 0, 2, "Actv", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_ATTACK, 0.005f, 20.0f, "AtkM", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_DECAY, 0.005f, 20.0f, "DcyM", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_SUSTAIN, 0.0f, 100.0f, "SusM", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::MOD_ADSR_RELEASE, 0.005f, 20.0f, "RlsM", Curve::EXPONENTIAL, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::NONE);
    ADD_PARAM(P::EFFECT_OVERDRIVE_DRIVE, 0.0f, 100.0f, "Drv", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_FREQ, 0.1f, 100.0f, "FrqC", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_DEPTH, 0.0f, 100.0f, "DptC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_FBK, 0.0f, 100.0f, "FbkC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_CHORUS_DELAY, 0.0f, 100.0f, "DlyC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_ATTACK, 0.001f, 10.0f, "AtkC", Curve::LINEAR, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_RELEASE, 0.001f, 10.0f, "RlsC", Curve::LINEAR, ParamUnit::SECONDS, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_THRESHOLD, -80.0f, 0.0f, "ThrC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_RATIO, 1.0f, 40.0f, "RatC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_COMPRESSOR_MAKEUP, 0.0f, 80.0f, "MkC", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_REVERB_FEEDBACK, 0.0f, 100.0f, "FbkR", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_REVERB_LPFREQ, 10.0f, 20000.0f, "CutxR", Curve::EXPONENTIAL, ParamUnit::HZ, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_1_DRYWET, 0.0f, 100.0f, "DrW1", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_1_ACTIVE, 0, 2, "Actv1", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::EFFECT_SLOT_2_DRYWET, 0.0f, 100.0f, "DrW2", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
    ADD_PARAM(P::EFFECT_SLOT_2_ACTIVE, 0, 2, "Actv2", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_MONO, 0, 2, "Mono", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_LEGATO, 0, 2, "Legato", Curve::LINEAR, ParamUnit::BOOL, ParamType::DISCRETE, UseInMain::NONE, UseInMod::NONE);
    ADD_PARAM(P::GLOBAL_PORTAMENTO, 0.0f, 100.0f, "Prtmnto", Curve::LINEAR, ParamUnit::PERCENT, ParamType::CONTINUOUS, UseInMain::USED, UseInMod::USED);
}

Modulator modulators[static_cast<int>(ModSource::COUNT_MOD_SOURCES)] = {
    {ModSource::NONE, 0.0f, "-"},
    {ModSource::LFO, 0.0f, "Lfo"},
    {ModSource::ADSR, 0.0f, "Adsr"},
    {ModSource::MOD_WHEEL, 0.0f, "Wheel"},
    {ModSource::AFTERTOUCH, 0.0f, "Aftch"},
};