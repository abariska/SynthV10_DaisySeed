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

const UWORD INTRO_PAGE_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD BG_BLACK_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD PARAM_BLOCK_SIZE = (((PARAM_BLOCK_WIDTH % 2 == 0) ? (PARAM_BLOCK_WIDTH / 2) : (PARAM_BLOCK_WIDTH / 2 + 1)) * PARAM_BLOCK_HEIGHT);
const UWORD WAVE_BUFFER_SIZE = (((WAVE_BUFFER_WIDTH % 2 == 0) ? (WAVE_BUFFER_WIDTH / 2) : (WAVE_BUFFER_WIDTH / 2 + 1)) * WAVE_BUFFER_HEIGHT);
const UWORD CPU_LOAD_BLOCK_SIZE = (((CPU_LOAD_BLOCK_WIDTH % 2 == 0) ? (CPU_LOAD_BLOCK_WIDTH / 2) : (CPU_LOAD_BLOCK_WIDTH / 2 + 1)) * CPU_LOAD_BLOCK_HEIGHT);
const UWORD PRESET_NAME_BLOCK_SIZE = (((PRESET_NAME_BLOCK_WIDTH % 2 == 0) ? (PRESET_NAME_BLOCK_WIDTH / 2) : (PRESET_NAME_BLOCK_WIDTH / 2 + 1)) * PRESET_NAME_BLOCK_HEIGHT);
const UWORD PRESET_NUM_BLOCK_SIZE = (((PRESET_NUM_BLOCK_WIDTH % 2 == 0) ? (PRESET_NUM_BLOCK_WIDTH / 2) : (PRESET_NUM_BLOCK_WIDTH / 2 + 1)) * PRESET_NUM_BLOCK_HEIGHT);

__attribute__((section(".sdram_bss"))) UBYTE intro_page[INTRO_PAGE_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE bg_black[BG_BLACK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE param_block[NUM_PARAM_BLOCKS][PARAM_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE wave_buffer[WAVE_BUFFER_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE cpu_load_block[CPU_LOAD_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE preset_name_block[PRESET_NAME_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE preset_num_block[PRESET_NUM_BLOCK_SIZE];  
 

ImageData intro_page_data;
ImageData bg_black_data;
ImageData param_block_data[NUM_PARAM_BLOCKS];
ImageData wave_buffer_data;
ImageData cpu_load_block_data;
ImageData preset_name_block_data;
ImageData preset_num_block_data;

void InitImages(){
    intro_page_data = {intro_page, INTRO_PAGE_SIZE};
    bg_black_data = {bg_black, BG_BLACK_SIZE};
    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        param_block_data[i] = {param_block[i], PARAM_BLOCK_SIZE};
    }
    wave_buffer_data = {wave_buffer, WAVE_BUFFER_SIZE};
    cpu_load_block_data = {cpu_load_block, CPU_LOAD_BLOCK_SIZE};
    preset_name_block_data = {preset_name_block, PRESET_NAME_BLOCK_SIZE};
    preset_num_block_data = {preset_num_block, PRESET_NUM_BLOCK_SIZE};
}

void DrawMainLines(){
    
    Paint_DrawLine(4, 34, 123, 34, 0x03, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(4, 36, 123, 36, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 58, 127, 58, 0x01, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    
    uint8_t x1 = 4;
    uint8_t x2 = 27;
    for(int i = 0; i < NUM_PARAM_BLOCKS; i++){
        // top line
        Paint_DrawLine(x1, BLOCK_TOP_LINE_Y, x2, BLOCK_TOP_LINE_Y, 0x01, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        // bottom line
        Paint_DrawLine(x1, BLOCK_BOTTOM_LINE_Y, x2, BLOCK_BOTTOM_LINE_Y, 0x01, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        x1 += 32;
        x2 += 32;
    }
}

void DrawFXLines(){
    Paint_DrawLine(4, 34, 123, 34, 0x03, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(4, 36, 123, 36, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 58, 127, 58, 0x01, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
}

void DrawParamPageLines(){
    Paint_DrawLine(4, 20, 123, 20, 0x03, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    
    uint8_t x1 = 4;
    uint8_t x2 = 27;
    for(int i = 0; i < NUM_PARAM_BLOCKS; i++){
        // top line
        Paint_DrawLine(x1, BLOCK_TOP_LINE_Y, x2, BLOCK_TOP_LINE_Y, 0x01, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        // bottom line
        Paint_DrawLine(x1, BLOCK_BOTTOM_LINE_Y, x2, BLOCK_BOTTOM_LINE_Y, 0x01, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        x1 += 32;
        x2 += 32;
    }
}

// void DrawPresetNumBlock(){

//     Paint_NewImage(preset_num_block, PRESET_NUM_BLOCK_WIDTH, PRESET_NUM_BLOCK_HEIGHT, 0, BLACK);
//     Paint_SetScale(16);
//     Paint_Clear(BLACK);
//     Paint_TextCentered("must B", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);
//     Paint_TextCentered("Program", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);
// }

// void DrawPresetNameBlock(uint8_t *block){
// }

void DrawIntroPage(){
    Paint_NewImage(intro_page_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);
    
    Paint_TextCentered("must B", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);
    Paint_TextCentered("by abariska", 64, FULL_PAGE_WIDTH, 112, Font8, WHITE, BLACK);
    
    OLED_Transmit_DMA(&intro_page_data);
}   
    
// void DrawIntroPage2(){
//     while (1)
//     {
//         static int i = 0;
//         char text[12];
//         Paint_NewImage(param_block_data[0].data, 32, 32, 0, BLACK);
//         Paint_Clear(BLACK);
    
//         sprintf(text, "%d", i);
//         Paint_TextCentered(text, 0, 32, 0, Font8, WHITE, BLACK);
//         Paint_TextCentered("by", 0, 32, 16, Font8, WHITE, BLACK);
//         OLED_Part_Transmit_DMA(&param_block_data[0], 40, 40, 72, 72);
//         i++;
//     }   
    
    // Paint_NewImage(param_block_data2.data, 32, 46, 0, BLACK);
    // Paint_Clear(WHITE);
    
    // Paint_TextCentered("B", 0, 32, 0, Font12, WHITE, BLACK);
    // Paint_TextCentered("by", 0, 32, 16, Font8, WHITE, BLACK);
    
    // OLED_Part_Transmit_DMA(&param_block_data2, 80, 80, 112, 106);
// }   

void DrawStaticPage(uint8_t color){
    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(color); 
    DrawMainLines();
}

void DrawStaticParamPage(uint8_t color){
    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(color); 
    DrawParamPageLines();
}


#endif
