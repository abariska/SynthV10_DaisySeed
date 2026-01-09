#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "OLED_Greyscale_Daisy/fonts.h"
#include "imageData.h"
#include "OLED_Greyscale_Daisy/GUI_Paint.h"
#include "OLED_Greyscale_Daisy/OLED_Driver.h"
#include "parameters.h"
#include "display.h"
#include "sx1509_expander.h"

#include <cstdint>

using namespace daisy;

const uint8_t yBlockLabel = 2;
const uint8_t yBlockValue = 20;
const uint8_t yBlockUnit = 38;

extern char page_name[16];
extern bool isBlink;
extern bool blinkStateChanged;
extern bool isStoreMode;
extern bool page_need_update;
extern bool shift_pressed;
extern bool isModMatrixNeedUpdate;
extern uint8_t selModBlockIndex;
extern uint8_t selSettingsBlockIndex;
extern bool isSettingsNeedUpdate;
extern bool updateStoreLed;
extern bool isStoreMode;

// Для перемикання між рядами параметрів
enum ActiveRow
{
    ROW_1 = 0, // Перший ряд (параметри 0-3)
    ROW_2 = 1  // Другий ряд (параметри 4-7)
};
extern ActiveRow currentActiveRow;

void UpdateEncoderSwitches();
void EditBlockParam(uint8_t blockIndex);
void UpdateEncodersParams();
void EncoderChangeEffect();
void EncoderChangeModMatrix();
void EncoderChangeSettings();
void DrawModMatrixBlocks();
void DrawModMatrixBlock(uint8_t blockIndex);
void EditModBlock();
void DrawMainBlocks();
void DrawParamBlocks();
void DrawOneParamBlock(uint8_t blockIndex, ParamUnitName target_param, uint16_t textColor = WHITE, uint16_t bgColor = BLACK);
void InitSlots();
void AssignParamsForPage(MenuPage page);
void SetPageName(const char *name);
void ToggleActiveRow();
void EncoderChangeStore();
void UpdateStoreLed();
void DrawWaveformImage(int waveform, bool custom_color = false, UBYTE color = 0xFF);
void DrawFilterModeText(int mode);

uint8_t GetActiveParamIndex(uint8_t encoderIndex); // Повертає індекс активного параметра для енкодера

#endif
