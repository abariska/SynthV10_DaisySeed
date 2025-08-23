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

struct ParamSlot {
    ParamUnitName target_param;
    bool need_update;   
};
extern ParamSlot slots[NUM_PARAM_BLOCKS];

struct MenuSlot {
    ParamUnitName target_param;
    bool need_update;
    bool isEditMode;
};
extern MenuSlot menu_slots[NUM_MAIN_SLOTS];

void UpdateEncoderSwitches();
void EditBlockParam(uint8_t blockIndex);
void UpdateParamValue(uint8_t encoderIndex, ParamUnitName target_param);
void UpdateMainParams();
void UpdateParamPageParams();
void UpdateEncodersParams();
void EncoderChangeEffect();

void InitMainBlocks();
void InitOneParamBlock(uint8_t blockIndex, ParamUnitName target_param, uint16_t textColor = WHITE, uint16_t bgColor = BLACK);
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