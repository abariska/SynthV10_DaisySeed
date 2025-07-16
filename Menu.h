#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "OLED_1.5_Daisy_Seed/ImageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "Parameters.h"
#include "Display.h"

#include <cstdint>

using namespace daisy;

extern char page_name[16];

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
    OSC_WAVEFORM_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_WAVEFORM_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
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
    EFFECT_CHORUS_PAN,
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

void EncoderChangeEffect();
void InitParam(ParamUnitName param, uint8_t slotIndex);
void InitParamBlocks();
void InitOneParamBlock(uint8_t blockIndex, float value, const char* label, uint16_t textColor = WHITE, uint16_t bgColor = BLACK);
void InitPageSlots();
void AssignParamsForPage(MenuPage page);
void SetPageName(const char* name);
void DrawPage(MenuPage page);
void DrawMainPage();
void DrawEffectsPage();
void DrawParamPage(MenuPage page);



#endif