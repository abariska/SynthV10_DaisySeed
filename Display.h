#ifndef OLED_H
#define OLED_H

#include "OLED_Greyscale_Daisy/OLED_Driver.h"
#include "OLED_Greyscale_Daisy/fonts.h"
#include "imageData.h"
#include "OLED_Greyscale_Daisy/GUI_Paint.h"
#include <cstdint>

#define PROGRAM_NAME_LENGTH 12
#define PROGRAM_NUMBER_LENGTH 4

#define PARAM_VALUE_LENGTH 8
#define FULL_PAGE_WIDTH DISPLAY_WIDTH // taken from Display_Config.h
#define FULL_PAGE_HEIGHT DISPLAY_HEIGHT // taken from Display_Config.h
#define PRESET_NUM_BLOCK_WIDTH FULL_PAGE_WIDTH
#define PRESET_NUM_BLOCK_HEIGHT 16
#define PARAM_BLOCK_WIDTH 64
#define PARAM_BLOCK_HEIGHT 50
#define EFFECT_BLOCK_WIDTH 128
#define EFFECT_BLOCK_HEIGHT 64
#define SCOPE_BLOCK_WIDTH FULL_PAGE_WIDTH
#define SCOPE_BLOCK_HEIGHT 50
#define MOD_MATRIX_BLOCK_WIDTH FULL_PAGE_WIDTH
#define MOD_MATRIX_BLOCK_HEIGHT 15
#define MOD_MATRIX_BLOCKS_NUM 7
#define SETTINGS_BLOCK_WIDTH FULL_PAGE_WIDTH
#define SETTINGS_BLOCK_HEIGHT 15
#define SETTINGS_BLOCKS_NUM 4
#define NUM_PARAM_BLOCKS 8
#define NUM_MAIN_SLOTS 4
#define NUM_FX_SLOTS 2
#define NUM_ACTIVE_PARAMS 4 // Скільки параметрів активні одночасно
#define WAVE_BUFFER_WIDTH 32
#define WAVE_BUFFER_HEIGHT 16
#define CPU_LOAD_BLOCK_WIDTH 20
#define CPU_LOAD_BLOCK_HEIGHT 16
#define STORE_BLOCK_WIDTH 128
#define STORE_BLOCK_HEIGHT 96
#define VOICES_BLOCK_WIDTH 20
#define VOICES_BLOCK_HEIGHT 16

#define BLOCK_WAVEFORM_Y_START 20
#define BLOCK_PARAM_Y_LABEL 0
#define BLOCK_PARAM_Y_VALUE 16
#define BLOCK_PARAM_Y_UNIT 32

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
    MOD_MATRIX_PAGE,
    SETTINGS_PAGE,
    EMPTY
};
extern MenuPage currentPage;

// Константи для двох рядів параметрів
#define BLOCK_ROW1_TOP_Y = 25;
#define BLOCK_ROW1_BOTTOM_Y = 70;
#define BLOCK_ROW2_TOP_Y = 78;
#define BLOCK_ROW2_BOTTOM_Y = 123;

const uint16_t BLOCK_X_START[] = {0, 64, 128, 192, 0, 64, 128, 192};
const uint16_t BLOCK_X_END[] = {64, 128, 192, 256, 64, 128, 192, 256};
const uint16_t BLOCK_Y_START[] = {28, 28, 28, 28, 78, 78, 78, 78};
const uint16_t BLOCK_Y_END[] = {78, 78, 78, 78, 128, 128, 128, 128};

const uint16_t BLOCK_MAIN_X_START[] = {0, 64, 128, 192};
const uint16_t BLOCK_MAIN_X_END[] = {64, 128, 192, 256};
const uint16_t BLOCK_MAIN_Y_START[] = {78, 78, 78, 78};
const uint16_t BLOCK_MAIN_Y_END[] = {128, 128, 128, 128};

const uint16_t BLOCK_FX_X_START[] = {0, 128};
const uint16_t BLOCK_FX_X_END[] = {128, 256};
const uint16_t BLOCK_FX_Y_START[] = {68, 68};
const uint16_t BLOCK_FX_Y_END[] = {128, 128};

const uint16_t BLOCK_MOD_MATRIX_X_START = 0;
const uint16_t BLOCK_MOD_MATRIX_X_END = 256;
const uint16_t BLOCK_MOD_MATRIX_Y_START[] = {28, 42, 56, 70, 84, 98, 112};
const uint16_t BLOCK_MOD_MATRIX_Y_END[] = {42, 56, 70, 84, 98, 112, 126};

const uint16_t BLOCK_SETTINGS_X_START = 0;
const uint16_t BLOCK_SETTINGS_X_END = 256;
const uint16_t BLOCK_SETTINGS_Y_START[] = {28, 42, 56, 70, 84, 98, 112};
const uint16_t BLOCK_SETTINGS_Y_END[] = {42, 56, 70, 84, 98, 112, 126};

const uint16_t BLOCK_SCOPE_X_START = 0;
const uint16_t BLOCK_SCOPE_X_END = 256;
const uint16_t BLOCK_SCOPE_Y_START = 26;
const uint16_t BLOCK_SCOPE_Y_END = 76;

const uint16_t BLOCK_STORE_X_START = 64;
const uint16_t BLOCK_STORE_X_END = 192;
const uint16_t BLOCK_STORE_Y_START = 16;
const uint16_t BLOCK_STORE_Y_END = 112;

const uint16_t BLOCK_LOAD_X_START = 0;
extern ImageData bg_black_data;
extern ImageData param_block_data[NUM_PARAM_BLOCKS];
extern ImageData wave_buffer_data;
extern ImageData effect_block_data[NUM_FX_SLOTS];
extern ImageData cpu_load_block_data;
extern ImageData preset_num_block_data;
extern ImageData mod_matrix_block_data[MOD_MATRIX_BLOCKS_NUM];
extern ImageData settings_block_data[SETTINGS_BLOCKS_NUM];
extern ImageData scope_block_data;
extern ImageData store_block_data;
extern ImageData voices_block_data;
void SetPage(MenuPage newPage);
void UpdatePage();
void DrawMainPage();
void DrawEffectsPage();
void DrawParamPage(MenuPage page);
void DrawIntroPage();
void SelectEffectPage(uint8_t slot);
void InitImages();
void DrawEffectBlock(uint8_t slot);
void DrawModMatrixPage();
void DrawScope();
void DrawModMatrixBlock(uint8_t blockIndex);
void DrawModMatrixBlocks();
void DrawWaveformImage(int waveform);
void DrawSettingsPage();
void DrawSettingsBlock(uint8_t blockIndex);
void DrawStoreBlock();

#endif
