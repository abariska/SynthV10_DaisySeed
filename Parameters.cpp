#include "parameters.h"
#include "daisy_seed.h"
#include "daisysp.h" // Add for using constants
#include "oscillator.h"
#include "display.h"
#include "log_uart.h"
#include "per/qspi.h"
#include "sys/dma.h"

using namespace daisy;
extern DaisySeed hw;
extern bool page_need_update;

static const size_t PRESET_SIZE = 256; // округлюємо до 512 байт
static const uint32_t FLASH_BASE_ADDR = 0x1000; // Починаємо пресети з 4KB
static const uint32_t FLASH_BLOCK_4KB = 0x1000;

Preset currentPreset;
float default_preset_array[(static_cast<int>(ParamUnitName::COUNT_PARAMS) - 1)] = { 0.0f,
    0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 1.0f, 
    0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 
    0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 
    0.5f, 0.0f, 0.1f, 0.1f, 1.0f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f,
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f
};

Preset GetDefaultPreset(int8_t presetNumber)
{
    Preset preset = {};
            preset.number = presetNumber;
            strcpy(preset.name, "default");
            memcpy(preset.array, default_preset_array, sizeof(default_preset_array));
            preset.type = PresetType::DEFAULT;
    return preset;
}

void ReadPreset(uint8_t preset_num, Preset &prst);
void SavePreset(uint8_t preset_num, const Preset &prst);
void ResetPreset(int presetNumber);

void InitQSPI()
{
    // Зчитуємо init_flag
    dsy_dma_invalidate_cache_for_buffer((uint8_t*)(0x90000000), 256);
    uint32_t init_flag = *((uint32_t*)(0x90000000));

    if(init_flag != 0xDEADBEEF)
    {
        hw.qspi.Erase(0, FLASH_BLOCK_4KB);

        uint8_t page[256];
        memset(page, 0xFF, sizeof(page));
        uint32_t marker = 0xDEADBEEF;
        memcpy(page, &marker, sizeof(marker));

        hw.qspi.Write(0, sizeof(page), page);

        System::Delay(10);

        dsy_dma_invalidate_cache_for_buffer((uint8_t*)(0x90000000), 256);
        hw.qspi.Erase(FLASH_BASE_ADDR, PRESET_NUM * FLASH_BLOCK_4KB);

        for(size_t i = 0; i <= PRESET_NUM; i++)
        {
            uint32_t addr = FLASH_BASE_ADDR + i * FLASH_BLOCK_4KB;
            Preset preset = GetDefaultPreset(i);
            uint8_t page[256];
            memset(page, 0, sizeof(page));
            memcpy(page, &preset, sizeof(preset));

            hw.qspi.Write(addr, sizeof(page), page);
            dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
        }
    }
}

// Continuous
SynthParameter::SynthParameter(float min_value, float max_value, const char* label, 
    uint8_t index, float* array, Curve defaultCurve, ParamUnit param_unit)
    : name_label(label), 
        param_index(index), 
        param_array(array), 
        min(min_value), 
        max(max_value), 
        curve(defaultCurve), 
        unit(param_unit), 
        type(ParamType::CONTINUOUS) 
    {}

// Discrete
SynthParameter::SynthParameter(int min_vals, int max_vals, 
    const char* label, uint8_t index, float* array, Curve defaultCurve, ParamUnit param_unit)
    : name_label(label),
      param_index(index),
      param_array(array),
      min(min_vals),
      max(max_vals),
      curve(defaultCurve),
      unit(param_unit),
      type(ParamType::DISCRETE)
    {}

void SynthParameter::SetFromCurrentPreset() {
    float n = currentPreset.array[param_index];  // Read from preset
    SetNormalized(n);  // Set normalized value
}

float SynthParameter::SetNormalized(float n) {
    n = clamp(n, 0.0f, 1.0f);
    norm_value = n;
    param_array[param_index] = norm_value;
    if (type == ParamType::DISCRETE) {
        physical_value = norm_value * max;  // 0..max
    } else {
        physical_value = min + norm_value * (max - min);  // min..max
    }
    return physical_value;
}

float SynthParameter::SetPhysicalValue(float v) {
    physical_value = clamp(v, min, max);
    if (type == ParamType::DISCRETE) {
        norm_value = physical_value / max;  // Ділимо на max, не на (max-1)
    } else {
        norm_value = (physical_value - min) / (max - min);
    }  
    param_array[param_index] = norm_value;
    return norm_value;
}

void SynthParameter::ModifyNormalized(float modifier) {

    norm_value += modifier;
    norm_value = clamp(norm_value, 0.0f, 1.0f);
}

float SynthParameter::AdjustByIncrement(int inc) {

    switch (unit) {
        case ParamUnit::HZ: {
            float newValue = 0.0f;
            if (physical_value >= 1.0f) {
                float ratio = powf(2.0f, inc / 12.0f);
                newValue = physical_value * ratio;
            } else {
                if (physical_value >= 0.1f) {
                float ratio = powf(2.0f, inc / 6.0f);
                newValue = physical_value * ratio;
                } else {
                    float ratio = powf(2.0f, inc / 2.0f);
                newValue = physical_value * ratio;
                }
            }
            return SetPhysicalValue(newValue);
        }
        case ParamUnit::SECONDS: {
        float newValue = 0.0f;
            if (physical_value >= 0.1f) {
                float ratio = powf(2.0f, inc / 12.0f);
                newValue = physical_value * ratio;
            } else {
                float ratio = powf(2.0f, inc / 6.0f);
                newValue = physical_value * ratio;
            }
            return SetPhysicalValue(newValue);
        }
        default: {
            float newValue  = physical_value + inc;
            return SetPhysicalValue(newValue);
        }
    }
}

// Getters
float SynthParameter::GetFloat() const { return static_cast<float>(physical_value); }
int SynthParameter::GetInt() const { 
    if (type == ParamType::DISCRETE) {
        if (norm_value < 0.001f) return 0;  // Явна обробка нуля
        return static_cast<int>(roundf(norm_value * (max - 1)));
    }
    return static_cast<int>(physical_value);
}
float SynthParameter::GetNormalised() const { return norm_value; }
bool SynthParameter::GetBool() const { return static_cast<float>(norm_value) > 0.5f; } 
const char* SynthParameter::GetLabel() const { return name_label; }
ParamType SynthParameter::GetType() const { return type; }
float SynthParameter::GetMin() const { return static_cast<float>(min); }
float SynthParameter::GetMax() const { return static_cast<float>(max); }
void SynthParameter::SetBool(bool value) { 
    norm_value = value > 0.5f ? 1.0f : 0.0f; 
    SetNormalized(norm_value);
}
ParamUnit SynthParameter::GetUnit() const { return unit; }
Curve SynthParameter::GetCurve() const { return curve; }

//--------------------------------
//--------------------------------

// Використання:
void InitSynthParams() {

    ReadPreset(0, currentPreset);

    paramManager.Init();
    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++) {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetFromCurrentPreset();
    } 
}
/** --- SavePreset --- */
void SavePreset(uint8_t preset_num, const Preset &prst)
{
    Preset p = prst;

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++) {
        float v = paramManager.GetParam(static_cast<ParamUnitName>(i)).GetNormalised();
        p.type = PresetType::CUSTOM;
        p.number = prst.number;
        strcpy(p.name, prst.name);
        p.array[i] = v;
    } 
    uint32_t addr = FLASH_BASE_ADDR + preset_num * FLASH_BLOCK_4KB;

    hw.qspi.Erase(addr, addr + FLASH_BLOCK_4KB);

    uint8_t page[256];
    memset(page, 0, sizeof(page));
    memcpy(page, &p, sizeof(p));

    hw.qspi.Write(addr, sizeof(page), page);
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    System::Delay(10);
}

/** --- ReadPreset --- */
void ReadPreset(uint8_t preset_num, Preset &prst)
{
    uint32_t addr = FLASH_BASE_ADDR + preset_num * FLASH_BLOCK_4KB;

    uint8_t page[256];
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    memcpy(&page, hw.qspi.GetData(addr), sizeof(page));
    memcpy(&prst, &page, sizeof(Preset));

    prst.name[11] = '\0';
    System::Delay(10);
}
void ResetPreset(int presetNumber){

    Preset preset = GetDefaultPreset(presetNumber);

    uint32_t addr = FLASH_BASE_ADDR + presetNumber * FLASH_BLOCK_4KB;

    hw.qspi.Erase(addr, addr + FLASH_BLOCK_4KB);

    uint8_t page[256];
    memset(page, 0, sizeof(page));
    memcpy(page, &preset, sizeof(preset));

    hw.qspi.Write(addr, sizeof(page), page);
    dsy_dma_invalidate_cache_for_buffer((uint8_t *)(0x90000000) + addr, sizeof(page));
    System::Delay(10);

    ReadPreset(presetNumber, currentPreset);

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++) {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetNormalized(currentPreset.array[i]);
    } 
    page_need_update = true;
    hw.DelayMs(10);
}

/** --- ApplyPreset --- */
void ApplyPreset(int presetNumber){
    
    ReadPreset(presetNumber, currentPreset);

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++) {
        paramManager.GetParam(static_cast<ParamUnitName>(i)).SetNormalized(currentPreset.array[i]);
    } 
    page_need_update = true;
}

ParameterManager paramManager;

void ParameterManager::Init() {
    using P = ParamUnitName;
// TODO: Finish with parameters
// TODO: Handle \n in labels
    params[static_cast<int>(P::NONE)] = SynthParameter(0, 0, "", 0, currentPreset.array);
    params[static_cast<int>(P::OSC_WAVEFORM_1)] = SynthParameter(0, Osc::WAVE_COUNT, "Wav", 1, currentPreset.array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_1)] = SynthParameter(-36.0f, 36.0f, "Sem", 2   , currentPreset.array, Curve::LINEAR, ParamUnit::SEMITONES); 
    params[static_cast<int>(P::OSC_DETUNE_1)] = SynthParameter(-100.0f, 100.0f, "Dtn", 3, currentPreset.array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_1)] = SynthParameter(0.0f, 100.0f, "Amp", 4, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_1)] = SynthParameter(-100.0f, 100.0f, "PWM", 5, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_1)] = SynthParameter(-100.0f, 100.0f, "Pan", 6, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_ACTIVE_1)] = SynthParameter(0, 2, "Actv", 7, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL);
    params[static_cast<int>(P::OSC_WAVEFORM_2)] = SynthParameter(0, Osc::WAVE_COUNT, "Wav", 8, currentPreset.array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_2)] = SynthParameter(-36.0f, 36.0f, "Sem", 9, currentPreset.array, Curve::LINEAR, ParamUnit::SEMITONES);
    params[static_cast<int>(P::OSC_DETUNE_2)] = SynthParameter(-100.0f, 100.0f, "Dtn", 10, currentPreset.array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_2)] = SynthParameter(0.0f, 100.0f, "Amp", 11, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_2)] = SynthParameter(-100.0f, 100.0f, "PWM", 12, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_2)] = SynthParameter(-100.0f, 100.0f, "Pan", 13, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_ACTIVE_2)] = SynthParameter(0, 2, "Actv", 14, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL);
    params[static_cast<int>(P::OSC_WAVEFORM_3)] = SynthParameter(0, Osc::WAVE_COUNT, "Wav", 15, currentPreset.array, Curve::LINEAR, ParamUnit::PICTURE);
    params[static_cast<int>(P::OSC_PITCH_3)] = SynthParameter(-36.0f, 36.0f, "Sem", 16, currentPreset.array, Curve::LINEAR, ParamUnit::SEMITONES);
    params[static_cast<int>(P::OSC_DETUNE_3)] = SynthParameter(-100.0f, 100.0f, "Dtn", 17, currentPreset.array, Curve::LINEAR, ParamUnit::CENTS);
    params[static_cast<int>(P::OSC_AMP_3)] = SynthParameter(0.0f, 100.0f, "Amp", 18, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PWM_3)] = SynthParameter(-100.0f, 100.0f, "PWM", 19, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::OSC_PAN_3)] = SynthParameter(-100.0f, 100.0f, "Pan", 20, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);    
    params[static_cast<int>(P::OSC_ACTIVE_3)] = SynthParameter(0, 2, "Actv", 21, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL); 
    params[static_cast<int>(P::FILTER_CUTOFF)] = SynthParameter(20.0f, 20000.0f, "Cut", 22, currentPreset.array, Curve::EXPONENTIAL, ParamUnit::HZ);
    params[static_cast<int>(P::FILTER_RESONANCE)] = SynthParameter(0.0f, 100.0f, "Res", 23, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::ADSR_ATTACK)] = SynthParameter( 0.005f, 10.0f, "Atk", 24, currentPreset.array, Curve::EXPONENTIAL, ParamUnit::SECONDS);
    params[static_cast<int>(P::ADSR_DECAY)] = SynthParameter(0.005f, 10.0f, "Dcy", 25, currentPreset.array, Curve::EXPONENTIAL, ParamUnit::SECONDS);
    params[static_cast<int>(P::ADSR_SUSTAIN)] = SynthParameter(0.0f, 100.0f, "Sus", 26, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::ADSR_RELEASE)] = SynthParameter(0.005f, 10.0f, "Rls", 27, currentPreset.array, Curve::EXPONENTIAL, ParamUnit::SECONDS);
    params[static_cast<int>(P::LFO_WAVEFORM)] = SynthParameter(0, Osc::WAVE_COUNT, "Wav", 28, currentPreset.array, Curve::LINEAR, ParamUnit::PICTURE); 
    params[static_cast<int>(P::LFO_FREQ)] = SynthParameter(0.01f, 50.0f, "Frq", 29, currentPreset.array, Curve::EXPONENTIAL, ParamUnit::HZ);
    params[static_cast<int>(P::LFO_DEPTH)] = SynthParameter( 0.0f, 100.0f, "Dpt", 30, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::LFO_ACTIVE)] = SynthParameter(0, 2, "Actv", 31, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL);
    params[static_cast<int>(P::EFFECT_OVERDRIVE_DRIVE)] = SynthParameter(0.0f, 100.0f, "Drv", 32, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_CHORUS_FREQ)] = SynthParameter(0.0f, 100.0f, "Frq", 33, currentPreset.array, Curve::LINEAR, ParamUnit::HZ);
    params[static_cast<int>(P::EFFECT_CHORUS_DEPTH)] = SynthParameter(0.0f, 1.0f, "Dpt", 34, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_CHORUS_FBK)] = SynthParameter(0.0f, 1.0f, "Fbk", 35, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_CHORUS_DELAY)] = SynthParameter(0.0f, 1.0f, "Dly", 36, currentPreset.array, Curve::LINEAR, ParamUnit::SECONDS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_ATTACK)] = SynthParameter(0.001f, 10.0f, "Atk", 37, currentPreset.array, Curve::LINEAR, ParamUnit::SECONDS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RELEASE)] = SynthParameter(0.001f, 10.0f, "Rls", 38, currentPreset.array, Curve::LINEAR, ParamUnit::SECONDS);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_THRESHOLD)] = SynthParameter(0.0f, -80.0f, "Thr", 39, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_RATIO)] = SynthParameter(1.0f, 40.0f, "Rat", 40, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_COMPRESSOR_MAKEUP)] = SynthParameter(0.0f, 80.0f, "Mk", 41, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_DRYWET)] = SynthParameter(0.0f, 1.0f, "DrW", 42, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_FEEDBACK)] = SynthParameter(0.0f, 1.0f, "Fbk", 43, currentPreset.array, Curve::LINEAR, ParamUnit::PERCENT);
    params[static_cast<int>(P::EFFECT_REVERB_LPFREQ)] = SynthParameter(0.0f, 1.0f, "LpF", 44, currentPreset.array, Curve::LINEAR, ParamUnit::HZ);
    params[static_cast<int>(P::GLOBAL_MONO)] = SynthParameter(0, 2, "Mon", 45, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL);
    params[static_cast<int>(P::GLOBAL_LEGATO)] = SynthParameter(0, 2, "Lgt", 46, currentPreset.array, Curve::LINEAR, ParamUnit::BOOL);
    params[static_cast<int>(P::GLOBAL_PORTAMENTO)] = SynthParameter(0.0f, 1.0f, "Prt", 47, currentPreset.array, Curve::LINEAR, ParamUnit::SECONDS);
}

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