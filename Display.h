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
#define PARAM_BLOCK_HEIGHT 64
#define SCOPE_BLOCK_WIDTH 128
#define SCOPE_BLOCK_HEIGHT 32
#define NUM_PARAM_BLOCKS 4

#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "OLED_1.5_Daisy_Seed/ImageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include <cstdint>

const uint8_t BLOCK_TOP_LINE_Y = 80;
const uint8_t BLOCK_BOTTOM_LINE_Y = 127;
const uint8_t BLOCK_X_START[] = {0, 32, 64, 96};
const uint8_t BLOCK_X_END[] = {32, 64, 96, 128};
const uint8_t BLOCK_LABEL_Y_START = 0;
const uint8_t BLOCK_VALUE_Y_START = 24;


UBYTE *background_black;
UWORD background_black_size = ((FULL_PAGE_WIDTH%2==0)? (FULL_PAGE_WIDTH/2)
    : (FULL_PAGE_WIDTH/2+1)) * FULL_PAGE_HEIGHT;

UBYTE *preset_num_block;
UWORD preset_num_black_size = ((PRESET_NUM_BLOCK_WIDTH%2==0)? (PRESET_NUM_BLOCK_WIDTH/2)
    : (PRESET_NUM_BLOCK_WIDTH/2+1)) * PRESET_NUM_BLOCK_HEIGHT;

UBYTE *preset_name_block;
UWORD preset_name_black_size = ((PRESET_NAME_BLOCK_WIDTH%2==0)? (PRESET_NAME_BLOCK_WIDTH/2)
    : (PRESET_NAME_BLOCK_WIDTH/2+1)) * PRESET_NAME_BLOCK_HEIGHT;

    
UBYTE *param_block[NUM_PARAM_BLOCKS];
UWORD param_block_size = ((PARAM_BLOCK_WIDTH%2==0)? (PARAM_BLOCK_WIDTH/2)
    : (PARAM_BLOCK_WIDTH/2+1)) * PARAM_BLOCK_HEIGHT;

UBYTE *intro_page;
UWORD intro_page_size = ((FULL_PAGE_WIDTH%2==0)? (FULL_PAGE_WIDTH/2)
    : (FULL_PAGE_WIDTH/2+1)) * FULL_PAGE_HEIGHT;

UBYTE *wave_buffer;
UWORD wave_buffer_size = ((32%2==0)? (32/2) : (32/2+1)) * 16; 

void InitImages(){

    if((background_black = (UBYTE *)malloc(background_black_size)) == NULL) {
        printf("Failed to allocate memory for background...\r\n");
        return; // Вихід з функції, якщо не вдалося виділити пам'ять
    }

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        if((param_block[i] = (UBYTE *)malloc(param_block_size)) == NULL) {
            printf("Failed to allocate memory for param block %zu...\r\n", i);
            return; // Вихід з функції, якщо не вдалося виділити пам'ять
        }
    }
    if((intro_page = (UBYTE *)malloc(intro_page_size)) == NULL) {
        printf("Failed to allocate memory for intro page\r\n");
        return; // Вихід з функції, якщо не вдалося виділити пам'ять
    }

    if((wave_buffer = (UBYTE *)malloc(wave_buffer_size)) == NULL) {
        printf("Failed to allocate memory for wave buffer\r\n");
        return; // Вихід з функції, якщо не вдалося виділити пам'ять
    }

    if((preset_name_block = (UBYTE *)malloc(preset_name_black_size)) == NULL) {
        printf("Failed to allocate memory for intro page\r\n");
        return; // Вихід з функції, якщо не вдалося виділити пам'ять
    }

    if((preset_num_block = (UBYTE *)malloc(preset_num_black_size)) == NULL) {
        printf("Failed to allocate memory for intro page\r\n");
        return; // Вихід з функції, якщо не вдалося виділити пам'ять
    }

    printf("Memory allocation successful\r\n");
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



void DrawParamBlock(uint8_t *block, const char* label, float value){
    Paint_SelectImage(block);
    Paint_SetScale(16);
    Paint_Clear(BLACK);
    char valStr[16];
    sprintf(valStr, "%d", (int)value);
    Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, 24, Font8, WHITE, BLACK);
    Paint_TextCentered(valStr, 0, PARAM_BLOCK_WIDTH, 48, Font8, WHITE, BLACK);
    OLED_1in5_Display_Part(block, 0, 0, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT);
}

void DrawPresetNumBlock(){

    Paint_NewImage(preset_num_block, PRESET_NUM_BLOCK_WIDTH, PRESET_NUM_BLOCK_HEIGHT, 0, BLACK);
    Paint_SetScale(16);
    Paint_Clear(BLACK);
    Paint_TextCentered("must B", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);
    Paint_TextCentered("Program", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);

}

void DrawPresetNameBlock(uint8_t *block){
}

void DrawCpuLoadBlock(uint8_t *block){
}

void InitDisplayPages(){
    OLED_1in5_Init();
    InitImages();
}

void DrawIntroPage(){
    Paint_NewImage(intro_page, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_SetScale(16);
    Paint_Clear(BLACK);
    
    Paint_TextCentered("must B", 0, FULL_PAGE_WIDTH, 50, Font24, WHITE, BLACK);
    Paint_TextCentered("by abariska", 64, FULL_PAGE_WIDTH, 112, Font8, WHITE, BLACK);
    
    OLED_1in5_Display(intro_page);
}   

void DrawStaticPage(uint8_t color){
    Paint_NewImage(background_black, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_SetScale(16); 
    Paint_Clear(color); 
    DrawMainLines();
}

void DrawStaticParamPage(uint8_t color){
    Paint_NewImage(background_black, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_SetScale(16); 
    Paint_Clear(color); 
    DrawParamPageLines();
}


#endif
