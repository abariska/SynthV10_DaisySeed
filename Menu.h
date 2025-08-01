#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "OLED_1.5_Daisy_Seed/ImageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "parameters.h"
#include "display.h"
#include "sx1509_expander.h"

#include <cstdint>

using namespace daisy;

extern char page_name[16];
extern bool isBlink;
extern bool blinkStateChanged;

// Для перемикання між рядами параметрів
enum ActiveRow {
    ROW_1 = 0,  // Перший ряд (параметри 0-3)
    ROW_2 = 1   // Другий ряд (параметри 4-7)
};
extern ActiveRow currentActiveRow;

enum MenuPage {
    MAIN_PAGE,
    OSCILLATOR_1_PAGE,
    OSCILLATOR_2_PAGE,
    OSCILLATOR_3_PAGE,
    FILTER_PAGE,
    AMPLIFIER_PAGE,
    LFO_PAGE,
    FX_PAGE,
    OVERDRIVE_PAGE,
    CHORUS_PAGE,
    COMPRESSOR_PAGE,
    REVERB_PAGE,
    MTX_PAGE,
    SETTINGS_PAGE,
    STORE_PAGE,
    LOAD_PAGE,
    EMPTY
};

extern MenuPage currentPage;

enum ParamUnitName {
    OSC_WAVEFORM_1,
    OSC_PITCH_1,
    OSC_DETUNE_1,
    OSC_AMP_1,
    OSC_PAN_1,
    OSC_WAVEFORM_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_PAN_2,
    OSC_WAVEFORM_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
    OSC_PAN_3,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    LFO_WAVEFORM,
    LFO_FREQ,
    LFO_DEPTH,
    EFFECT_OVERDRIVE_DRIVE,
    EFFECT_CHORUS_FREQ,
    EFFECT_CHORUS_DEPTH,
    EFFECT_CHORUS_FBK,
    EFFECT_CHORUS_DELAY,
    EFFECT_COMPRESSOR_ATTACK,
    EFFECT_COMPRESSOR_RELEASE,
    EFFECT_COMPRESSOR_THRESHOLD,
    EFFECT_COMPRESSOR_RATIO,
    EFFECT_REVERB_DRYWET,
    EFFECT_REVERB_FBK,
    EFFECT_REVERB_LPFREQ,
    NONE
};

struct ParamUnitData{
    float* target_param;
    const char* label;
    float min;
    float max;
    float sensitivity;
    ValueType valueType;
} ;
extern ParamUnitData allParams[ParamUnitName::NONE + 1];

struct ParamSlot {
    ParamUnitName assignedParam;
    bool need_update;   
};
extern ParamSlot slots[NUM_PARAM_BLOCKS];

struct MenuSlot {
    ParamUnitName assignedParam;
    bool need_update;
    bool isEditMode;
};
extern MenuSlot menu_slots[NUM_MAIN_SLOTS];

void UpdateEncoderSwitches();
void EditBlockParam(uint8_t blockIndex);
void UpdateParamValue(uint8_t encoderIndex, ParamUnitName paramName, float* target_param);
void UpdateMainParams();
void UpdateParamPageParams();
void UpdateEncodersParams();
void EncoderChangeEffect();

void AssignParam(ParamUnitName param, uint8_t slotIndex);
void AssignMainParams();
void InitMainBlocks();
void InitOneParamBlock(uint8_t blockIndex, float value, const char* label, uint16_t textColor = WHITE, uint16_t bgColor = BLACK);
void InitSlots();
void AssignParamsForPage(MenuPage page);
void SetPageName(const char* name);
void DrawPage(MenuPage page);
void DrawMainPage();
void DrawEffectsPage();
void DrawParamPage(MenuPage page);
void ToggleActiveRow();
void InitParamBlocks();
uint8_t GetActiveParamIndex(uint8_t encoderIndex);  // Повертає індекс активного параметра для енкодера
void UpdateBlinking(uint8_t blockIndex);


#endif