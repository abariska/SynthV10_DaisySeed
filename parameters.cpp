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
ParameterManager paramManager;
AudioParamsDirty dirty;
const float default_preset_values[(static_cast<int>(ParamUnitName::COUNT_PARAMS))] = {0.0f,
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
    memcpy(preset.values, default_preset_values, sizeof(default_preset_values));

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

    if (init_flag != 0xDEADBEEC)
    {
        hw.qspi.Erase(0, FLASH_BLOCK_4KB);

        uint8_t page[PAGE_SIZE];
        memset(page, 0xFF, sizeof(page));
        uint32_t marker = 0xDEADBEEC;
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
// TODO
void ParameterManager::Init()
{
    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        dirty[i] = true;
    }
}

float ParameterManager::SetNormalized(ParamUnitName name, float n)
{
    int param_index = static_cast<int>(name);
    n = clamp(n, 0.0f, 1.0f);
    values[param_index].normal = n;
    if (desc[param_index].type == ParamType::DISCRETE)
    {
        if (desc[param_index].unit == ParamUnit::BOOL)
        {
            values[param_index].physical = values[param_index].normal > 0.5f ? 1.0f : 0.0f;
        }
        else
        {
            int num_values = static_cast<int>(desc[param_index].max - desc[param_index].min) + 1;
            float exact_value = desc[param_index].min + values[param_index].normal * (num_values - 1);
            values[param_index].physical = roundf(exact_value);
            values[param_index].normal = (values[param_index].physical - desc[param_index].min) / (num_values - 1);
        }
    }
    else
    {
        values[param_index].physical = desc[param_index].min + values[param_index].normal * (desc[param_index].max - desc[param_index].min); // min..max
    }
    return values[param_index].physical;
}

float ParameterManager::SetPhysicalValue(ParamUnitName name, float v)
{
    int param_index = static_cast<int>(name);
    values[param_index].physical = clamp(v, desc[param_index].min, desc[param_index].max);
    if (desc[param_index].type == ParamType::DISCRETE)
    {
        if (desc[param_index].unit == ParamUnit::BOOL)
        {
            values[param_index].physical = v > 0.5f ? 1.0f : 0.0f;
            values[param_index].normal = values[param_index].physical > 0.5f ? 1.0f : 0.0f;
        }
        else
        {
        values[param_index].physical = roundf(values[param_index].physical);
        int num_values = static_cast<int>(desc[param_index].max - desc[param_index].min) + 1;
            values[param_index].normal = (values[param_index].physical - desc[param_index].min) / (num_values - 1);
        }
    }
    else
    {
        values[param_index].normal = (values[param_index].physical - desc[param_index].min) / (desc[param_index].max - desc[param_index].min);
    }
    return values[param_index].normal;
}

void ParameterManager::SetDirty(ParamUnitName name, bool isDirty)
{
    int param_index = static_cast<int>(name);
    dirty[param_index] = isDirty;
}

float ParameterManager::AdjustByIncrement(ParamUnitName name, int inc)
{
    int param_index = static_cast<int>(name);
    ParamUnit unit = desc[param_index].unit;
    ParamType type = desc[param_index].type;
    SetAudioDirtyFlag(name);
    if (type == ParamType::DISCRETE )
    {
        if (unit == ParamUnit::BOOL)
        {
            float newValue = static_cast<float>(values[param_index].normal);
            newValue += inc;
            SetNormalized(name, newValue);
            return static_cast<float>(newValue);
        }
        else
        {
            int current_int = static_cast<int>(values[param_index].physical);
            int new_int = current_int + inc;
            return SetPhysicalValue(name, static_cast<float>(new_int));
        }
    }

    switch (unit)
    {
    case ParamUnit::HZ:
    case ParamUnit::FREQ:
    {
        float newValue = 0.0f;
        if (values[param_index].physical >= 1.0f)
        {
            float ratio = powf(2.0f, inc / 12.0f);
            newValue = values[param_index].physical * ratio;
        }
        else
        {
            if (values[param_index].physical >= 0.1f)
            {
                float ratio = powf(2.0f, inc / 6.0f);
                newValue = values[param_index].physical * ratio;
            }
            else
            {
                float ratio = powf(2.0f, inc / 2.0f);
                newValue = values[param_index].physical * ratio;
            }
        }
        return SetPhysicalValue(name, newValue);
    }
    case ParamUnit::SECONDS:
    {
        float newValue = 0.0f;
        if (values[param_index].physical >= 0.1f)
        {
            float ratio = powf(2.0f, inc / 12.0f);
            newValue = values[param_index].physical * ratio;
        }
        else
        {
            float ratio = powf(2.0f, inc / 6.0f);
            newValue = values[param_index].physical * ratio;
        }
        return SetPhysicalValue(name, newValue);
    }
    default:
    {
        float newValue = values[param_index].physical + inc;
        return SetPhysicalValue(name, newValue);
    }
    }
}

float ParameterManager::GetValue(ParamUnitName name) const
{
    int param_index = static_cast<int>(name);
    float value = 0.0f;
    float m = desc[param_index].max;
    ParamUnit unit = desc[param_index].unit;
    float mod_value = (desc[param_index].flags & ~FLAG_PER_VOICE) ? mod[param_index].mod_global : 0.0f;

    switch (unit)
    {
    case ParamUnit::FREQ:
        value = values[param_index].physical;
        return value + (value * mod_value);
        break;
    case ParamUnit::HZ:
    case ParamUnit::SECONDS:
        value = values[param_index].physical;
        return value + ((m - value) * mod_value);
        break;
    
    case ParamUnit::SEMITONES:
    case ParamUnit::CENTS:
    case ParamUnit::PICTURE:
    case ParamUnit::TEXT:
    case ParamUnit::UNITLESS:
        return GetInt(name);
        break;
    case ParamUnit::PERCENT:
        value = values[param_index].normal;
        m = 1.0f;
        return value + ((m - value) * mod_value);
        break;
    case ParamUnit::BOOL:
        return static_cast<float>(values[param_index].normal > 0.5f ? 1.0f : 0.0f);
        break;
    default:
        return 0.0f;
        break;
    }
}

float ParameterManager::GetParamUpdate(ParamUnitName name) const
{
    int param_index = static_cast<int>(name);
    if (dirty[param_index])
    {
        return GetValue(name);
    }
}

int ParameterManager::GetInt(ParamUnitName name) const
{
    int param_index = static_cast<int>(name);
    if (desc[param_index].type == ParamType::DISCRETE)
    {
        int range = static_cast<int>(desc[param_index].max - desc[param_index].min);
        int offset = static_cast<int>(roundf(values[param_index].normal * range));
        return static_cast<int>(desc[param_index].min) + offset;
    }
    return static_cast<int>(values[param_index].physical);
}

void ParameterManager::SetBool(ParamUnitName name, bool value)
{
    float val = value ? 1.0f : 0.0f;
    SetNormalized(name, val);
}

void ParameterManager::SetFromCurrentPreset(ParamUnitName name) 
{ 
    values[static_cast<int>(name)].normal = currentPreset.values[static_cast<int>(name)]; 
    paramManager.SetNormalized(name, values[static_cast<int>(name)].normal);
}

//--------------------------------
//--------------------------------

void InitSynthParams()
{

    ReadPreset(0, currentPreset);

    paramManager.Init();
    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        paramManager.SetFromCurrentPreset(static_cast<ParamUnitName>(i));
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
        p.values[i] = paramManager.GetNormalised(static_cast<ParamUnitName>(i));
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
        paramManager.SetNormalized(static_cast<ParamUnitName>(i), currentPreset.values[i]);
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

    ResetModModulators();

    for (size_t i = 0; i < static_cast<int>(ParamUnitName::COUNT_PARAMS); i++)
    {
        ParamUnitName param = static_cast<ParamUnitName>(i);
        paramManager.SetNormalized(param, currentPreset.values[i]);
        paramManager.SetModifier(param, 0.0f);
        for (size_t v = 0; v < VOICE_NUM; v++)
        {
            paramManager.SetModifierPerVoice(param, v, 0.0f);
        }
    }
    DirtyFlagsToTrue();
    page_need_update = true;
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

void ResetModModulators()
{
    for (size_t i = 0; i < static_cast<int>(ModSource::COUNT_MOD_SOURCES); i++)
    {
        modulators[i].value = 1.0f;
        for (size_t j = 0; j < VOICE_NUM; j++)
        {
            modulators[i].value_per_voice[j] = 0.0f;
        }
    }
}

void SetAudioDirtyFlag(ParamUnitName param) {

    paramManager.SetDirty(param, true);

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