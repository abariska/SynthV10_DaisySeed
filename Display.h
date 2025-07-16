#ifndef OLED_H
#define OLED_H

#define PROGRAM_NAME_LENGTH 12
#define PROGRAM_NUMBER_LENGTH 4
#define PARAM_NAME_LENGTH 8
#define PARAM_VALUE_LENGTH 8
#define FULL_PAGE_WIDTH 128
#define FULL_PAGE_HEIGHT 128
#define PRESET_NAME_BLOCK_WIDTH 128
#define PRESET_NAME_BLOCK_HEIGHT 16
#define PRESET_NUM_BLOCK_WIDTH 128
#define PRESET_NUM_BLOCK_HEIGHT 16
#define PARAM_BLOCK_WIDTH 32
#define PARAM_BLOCK_HEIGHT 46
#define OSC_ON_BLOCK_WIDTH 16
#define OSC_ON_BLOCK_HEIGHT 16
#define SCOPE_BLOCK_WIDTH 128
#define SCOPE_BLOCK_HEIGHT 32
#define NUM_PARAM_BLOCKS 4
#define WAVE_BUFFER_WIDTH 32
#define WAVE_BUFFER_HEIGHT 16
#define CPU_LOAD_BLOCK_WIDTH 16 
#define CPU_LOAD_BLOCK_HEIGHT 16

#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "OLED_1.5_Daisy_Seed/ImageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include <cstdint>

const uint8_t BLOCK_TOP_LINE_Y = 80;
const uint8_t BLOCK_BOTTOM_LINE_Y = 126;
const uint8_t BLOCK_X_START[] = {0, 32, 64, 96};
const uint8_t BLOCK_X_END[] = {32, 64, 96, 128};
const uint8_t BLOCK_LABEL_Y_START = 0;
const uint8_t BLOCK_VALUE_Y_START = 24;


extern ImageData intro_page_data;
extern ImageData bg_black_data;
extern ImageData param_block_data[NUM_PARAM_BLOCKS];
extern ImageData wave_buffer_data;
extern ImageData osc_on_block_data;
extern ImageData cpu_load_block_data;
extern ImageData preset_name_block_data;
extern ImageData preset_num_block_data;



#endif
