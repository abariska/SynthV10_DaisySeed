#ifndef OLED_H
#define OLED_H

#define PROGRAM_NAME_LENGTH 12
#define PROGRAM_NUMBER_LENGTH 4

#define PARAM_VALUE_LENGTH 8
#define FULL_PAGE_WIDTH 128
#define FULL_PAGE_HEIGHT 128
#define PRESET_NAME_BLOCK_WIDTH 128
#define PRESET_NAME_BLOCK_HEIGHT 16
#define PRESET_NUM_BLOCK_WIDTH 128
#define PRESET_NUM_BLOCK_HEIGHT 16
#define PARAM_BLOCK_WIDTH 32
#define PARAM_BLOCK_HEIGHT 50
#define OSC_ON_BLOCK_WIDTH 16
#define OSC_ON_BLOCK_HEIGHT 16
#define SCOPE_BLOCK_WIDTH 128
#define SCOPE_BLOCK_HEIGHT 32
#define NUM_PARAM_BLOCKS 8
#define NUM_MAIN_SLOTS 4
#define NUM_ACTIVE_PARAMS 4 // Скільки параметрів активні одночасно
#define WAVE_BUFFER_WIDTH 32
#define WAVE_BUFFER_HEIGHT 16
#define CPU_LOAD_BLOCK_WIDTH 16
#define CPU_LOAD_BLOCK_HEIGHT 16

#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "imageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include <cstdint>

enum MenuPage
{
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

// Константи для двох рядів параметрів
const uint8_t BLOCK_ROW1_TOP_Y = 25;
const uint8_t BLOCK_ROW1_BOTTOM_Y = 70;
const uint8_t BLOCK_ROW2_TOP_Y = 78;
const uint8_t BLOCK_ROW2_BOTTOM_Y = 123;

const uint8_t BLOCK_X_START[] = {0, 32, 64, 96, 0, 32, 64, 96};
const uint8_t BLOCK_X_END[] = {32, 64, 96, 128, 32, 64, 96, 128};
const uint8_t BLOCK_Y_START[] = {24, 24, 24, 24, 74, 74, 74, 74};
const uint8_t BLOCK_Y_END[] = {74, 74, 74, 74, 124, 124, 124, 124};

const uint8_t BLOCK_MAIN_X_START[] = {0, 32, 64, 96};
const uint8_t BLOCK_MAIN_X_END[] = {32, 64, 96, 128};
const uint8_t BLOCK_MAIN_Y_START[] = {74, 74, 74, 74};
const uint8_t BLOCK_MAIN_Y_END[] = {124, 124, 124, 124};

extern ImageData intro_page_data;
extern ImageData bg_black_data;
extern ImageData param_block_data[NUM_PARAM_BLOCKS];
extern ImageData wave_buffer_data;
extern ImageData osc_on_block_data;
extern ImageData cpu_load_block_data;
extern ImageData preset_name_block_data;
extern ImageData preset_num_block_data;

void SetPage(MenuPage newPage);
void UpdatePage();
void DrawMainPage();
void DrawEffectsPage();
void DrawParamPage(MenuPage page);
void DrawIntroPage();
void SelectEffectPage(uint8_t slot);
void InitImages();

#endif
