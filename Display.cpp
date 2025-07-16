
#include "Display.h"
#include "Menu.h"

extern MenuPage currentPage;

const UWORD INTRO_PAGE_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD BG_BLACK_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD PARAM_BLOCK_SIZE = (((PARAM_BLOCK_WIDTH % 2 == 0) ? (PARAM_BLOCK_WIDTH / 2) : (PARAM_BLOCK_WIDTH / 2 + 1)) * PARAM_BLOCK_HEIGHT);
const UWORD WAVE_BUFFER_SIZE = (((WAVE_BUFFER_WIDTH % 2 == 0) ? (WAVE_BUFFER_WIDTH / 2) : (WAVE_BUFFER_WIDTH / 2 + 1)) * WAVE_BUFFER_HEIGHT);
const UWORD OSC_ON_BLOCK_SIZE = (((OSC_ON_BLOCK_WIDTH % 2 == 0) ? (OSC_ON_BLOCK_WIDTH / 2) : (OSC_ON_BLOCK_WIDTH / 2 + 1)) * OSC_ON_BLOCK_HEIGHT);
const UWORD CPU_LOAD_BLOCK_SIZE = (((CPU_LOAD_BLOCK_WIDTH % 2 == 0) ? (CPU_LOAD_BLOCK_WIDTH / 2) : (CPU_LOAD_BLOCK_WIDTH / 2 + 1)) * CPU_LOAD_BLOCK_HEIGHT);
const UWORD PRESET_NAME_BLOCK_SIZE = (((PRESET_NAME_BLOCK_WIDTH % 2 == 0) ? (PRESET_NAME_BLOCK_WIDTH / 2) : (PRESET_NAME_BLOCK_WIDTH / 2 + 1)) * PRESET_NAME_BLOCK_HEIGHT);
const UWORD PRESET_NUM_BLOCK_SIZE = (((PRESET_NUM_BLOCK_WIDTH % 2 == 0) ? (PRESET_NUM_BLOCK_WIDTH / 2) : (PRESET_NUM_BLOCK_WIDTH / 2 + 1)) * PRESET_NUM_BLOCK_HEIGHT);

__attribute__((section(".sdram_bss"))) UBYTE intro_page[INTRO_PAGE_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE bg_black[BG_BLACK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE param_block[NUM_PARAM_BLOCKS][PARAM_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE wave_buffer[WAVE_BUFFER_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE osc_on_block[OSC_ON_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE cpu_load_block[CPU_LOAD_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE preset_name_block[PRESET_NAME_BLOCK_SIZE];
__attribute__((section(".sdram_bss"))) UBYTE preset_num_block[PRESET_NUM_BLOCK_SIZE];  

ImageData intro_page_data;
ImageData bg_black_data;
ImageData param_block_data[NUM_PARAM_BLOCKS];
ImageData wave_buffer_data;
ImageData osc_on_block_data;
ImageData cpu_load_block_data;
ImageData preset_name_block_data;
ImageData preset_num_block_data;

void InitImages(){

    memset(intro_page, 0, INTRO_PAGE_SIZE);
    memset(bg_black, 0, BG_BLACK_SIZE);
    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        memset(param_block[i], 0, PARAM_BLOCK_SIZE);
    }
    memset(wave_buffer, 0, WAVE_BUFFER_SIZE);
    memset(osc_on_block, 0, OSC_ON_BLOCK_SIZE);
    memset(cpu_load_block, 0, CPU_LOAD_BLOCK_SIZE);
    memset(preset_name_block, 0, PRESET_NAME_BLOCK_SIZE);
    memset(preset_num_block, 0, PRESET_NUM_BLOCK_SIZE);
    
    intro_page_data = {intro_page, INTRO_PAGE_SIZE};
    bg_black_data = {bg_black, BG_BLACK_SIZE};
    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        param_block_data[i] = {param_block[i], PARAM_BLOCK_SIZE};
    }
    wave_buffer_data = {wave_buffer, WAVE_BUFFER_SIZE};
    osc_on_block_data = {osc_on_block, OSC_ON_BLOCK_SIZE};
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

void SetPage(MenuPage newPage) {

    if (currentPage == newPage) return;

    currentPage = newPage;    

    DrawPage(newPage);
}

void DrawPage(MenuPage page) {

    switch (page)
    {
    case MAIN_PAGE:
        DrawMainPage();
        break;
    case FX_PAGE:
        DrawEffectsPage();
        break;
    default:
        DrawParamPage(page);
        break;
    }
}

void DrawMainPage()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];

    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK); 
    DrawMainLines();
    
    sprintf(prog_num, "%03d", 1);
    Paint_TextCentered(prog_num, 0, 127, 0, Font16, WHITE, BLACK);

    sprintf(prog_name, "Program");
    Paint_TextCentered(prog_name, 0, 127, 16, Font16, WHITE, BLACK);

    AssignParamsForPage(MAIN_PAGE); 

    OLED_Transmit_DMA(&bg_black_data);
    InitParamBlocks();
}

void DrawParamPage(MenuPage page){

    AssignParamsForPage(page); 

    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK); 

    DrawParamPageLines();

    Paint_TextCentered(page_name, 0, 127, 0, Font12, WHITE, BLACK);
    OLED_Transmit_DMA(&bg_black_data);

    InitParamBlocks();
}  