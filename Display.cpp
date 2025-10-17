
#include "display.h"
#include "menu.h"
#include "effects.h"
#include "parameters.h"
#include "voice.h"

#define font8 u8g2_font_5x8_tf;
#define font10 u8g2_font_6x10_tf;
#define font12 u8g2_font_6x12_tf;
#define font14 u8g2_font_7x14_tf;
#define font15 u8g2_font_9x15_tf;
#define font20 u8g2_font_10x20_tf;

extern Preset currentPreset;
extern float scope_data[128];
extern int scope_data_index;
extern bool scope_data_ready;


MenuPage currentPage = MAIN_PAGE;

bool scope_draw = false;

SpiHandle spi_display;
SpiHandle::Config spi_config;
GPIO pin_dc;
GPIO pin_reset;
GPIO pin_cs;
u8g2_t myDisplay;
/*------------------------------------------------------------------------------------------------------*/

void SPI_Config()
{
    
    // SPI peripheral config
    spi_config.periph = SpiHandle::Config::Peripheral::SPI_1;
    spi_config.mode   = SpiHandle::Config::Mode::MASTER;
    spi_config.direction
        = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
    spi_config.datasize       = 8;
    spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
    spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
    spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;
    // SPI pin config
    spi_config.pin_config.sclk = Pin(PORTG, 11);
    spi_config.pin_config.mosi = Pin(PORTA, 7);
    spi_config.pin_config.nss  = Pin(PORTG, 10);
    spi_display.Init(spi_config);
}

void SPI_Init()
{
    SPI_Config();
    // SSD1327 control pin config
    pin_dc.Init(Pin(PORTC, 1), GPIO::Mode::OUTPUT);
    pin_reset.Init(Pin(PORTB, 1), GPIO::Mode::OUTPUT);
    pin_cs.Init(Pin(PORTG, 10), GPIO::Mode::OUTPUT);
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	  switch(msg)
	  {
	  case U8X8_MSG_DELAY_MILLI:
		  System::Delay(arg_int);
		  break;
	  case U8X8_MSG_GPIO_CS:
		  pin_cs.Write(arg_int);
		  break;
	  case U8X8_MSG_GPIO_DC:
		  pin_dc.Write(arg_int);
		  break;
	  case U8X8_MSG_GPIO_RESET:
		  pin_reset.Write(arg_int);
		  break;
	  }
	  return 1;
}

uint8_t u8x8_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	  switch(msg)
	  {
	  case U8X8_MSG_BYTE_SET_DC:
		  pin_dc.Write(arg_int);
		  break;
	  case U8X8_MSG_BYTE_SEND:
		  spi_display.DmaTransmit((uint8_t *)arg_ptr, arg_int, NULL, NULL, NULL);
		  break;
	  case U8X8_MSG_BYTE_START_TRANSFER:
		  pin_cs.Write(0);
		  break;
	  case U8X8_MSG_BYTE_END_TRANSFER:
		  pin_cs.Write(1);
		  break;
	  }
	  return 1;
}

void ssd1327_Init()
{
    SPI_Init();
    u8g2_Setup_ssd1327_ws_128x128_1(&myDisplay, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&myDisplay);
    u8g2_SetPowerSave(&myDisplay, 0);
}

void Paint_TextCentered(const char* text, uint8_t x1, uint8_t x2, uint8_t y, const uint8_t *font, uint16_t text_color, uint16_t background_color) {
    
    u8g2_SetFont(&myDisplay, font);
    u8g2_SetDrawColor(&myDisplay, text_color);
    u8g2_DrawButtonUTF8(&myDisplay, x1, y, U8G2_BTN_BW0, x2 - x1, 0, 0, text);
}

void Paint_NumCentered(int param, uint8_t x1, uint8_t x2, uint8_t y, uint8_t Digit, const uint8_t *font, uint16_t text_color, uint16_t background_color){
    
    u8g2_SetFont(&myDisplay, font);
    u8g2_SetDrawColor(&myDisplay, text_color);
    u8g2_DrawButtonUTF8(&myDisplay, x1, y, U8G2_BTN_BW0, x2 - x1, 0, 0, std::to_string(param).c_str());
    
}

void SetPage(MenuPage newPage)
{

    if (currentPage == newPage)
    {
        return;
    }

    currentPage = newPage;
    currentActiveRow = ROW_1;

    switch (newPage)
    {
    case MAIN_PAGE:
        DrawMainPage();
        break;
    case FX_PAGE:
        DrawEffectsPage();
        break;
    case MOD_MATRIX_PAGE:
        DrawModMatrixPage();
        break;
    case SETTINGS_PAGE:
        DrawSettingsPage();
        break;
    default:
        DrawParamPage(newPage);
        break;
    }
    UpdateLeds();
    if (currentPage != MAIN_PAGE)
    {
        for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
        {
            currentPreset.mainSlots[i].isEditMode = false;
        }
    }
}
void UpdatePage()
{
    if (!page_need_update)
    {
        return;
    }
    switch (currentPage)
    {
    case MAIN_PAGE:
        DrawMainPage();
        scope_draw = true;
        break;
    case FX_PAGE:
        DrawEffectsPage();
        break;
    case MOD_MATRIX_PAGE:
        DrawModMatrixPage();
        break;
    case SETTINGS_PAGE:
        DrawSettingsPage();
        break;
    default:
        DrawParamPage(currentPage);
        break;
    }
    UpdateLeds();
    if (currentPage != MAIN_PAGE)
    {
        for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
        {
            currentPreset.mainSlots[i].isEditMode = false;
        }
    }
    page_need_update = false;
}

void DrawScope()
{
    if (currentPage != MAIN_PAGE || isStoreMode)
    {
        return;
    }
    
    double time1 = System::GetNow();
    static double time_end = 0;
    
    if (time1 - time_end > 30)
    {
        if (!scope_data_ready)
        {
            return;
        }
        u8g2_ClearDisplay(&myDisplay);
        float max_val = 0.0f;
        for (int i = 0; i < 128; i++)
        {
            float abs_val = fabs(scope_data[i]);
            if (abs_val > max_val) max_val = abs_val;
        }
        
        if (max_val < 0.001f) max_val = 0.001f;
        
        // float scale = (SCOPE_BLOCK_HEIGHT * 0.5f) / max_val;  // 80% висоти екрану
        int center_y = SCOPE_BLOCK_HEIGHT / 2;
        
        for (int i = 0; i < SCOPE_BLOCK_WIDTH - 1; i++)
        {
            int data_index = (i * 128) / SCOPE_BLOCK_WIDTH;  // Інтерполяція
            int y = center_y - (int)(scope_data[data_index] * 30);
            
            // Обмежуємо координати
            if (y < 0) y = 0;
            if (y >= SCOPE_BLOCK_HEIGHT) y = SCOPE_BLOCK_HEIGHT - 1;
            
            u8g2_DrawPixel(&myDisplay, i, y);
        }
        
        scope_data_ready = false;

        u8g2_SendBuffer(&myDisplay);
        time_end = time1;
    }
}

void DrawMainPage()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    // char prog_name[PROGRAM_NAME_LENGTH];

    u8g2_ClearDisplay(&myDisplay);

    u8g2_SetDrawColor(&myDisplay, 1);
    u8g2_DrawLine(&myDisplay, 5, 18, 123, 18);
    u8g2_DrawLine(&myDisplay, 5, 20, 123, 20);

    sprintf(prog_num, "%03d", currentPreset.number);
    u8g2_SetFont(&myDisplay, u8g2_font_ncenB14_tr);
    u8g2_SetDrawColor(&myDisplay, 0);
    u8g2_DrawButtonUTF8(&myDisplay, 0, 127, 0, U8G2_BTN_BW0, 0, 0, prog_num); 

    // sprintf(prog_name, "%s", currentPreset.name);
    // Paint_TextCentered(prog_name, 0, 127, 16, Font16, WHITE, BLACK);

    u8g2_SendBuffer(&myDisplay);
    DrawMainBlocks();
    DrawScope();
}

void DrawParamPage(MenuPage page)
{

    AssignParamsForPage(page);

    u8g2_ClearDisplay(&myDisplay);
    u8g2_SetDrawColor(&myDisplay, 1);
    u8g2_DrawLine(&myDisplay, 4, 22, 123, 22);
    u8g2_DrawLine(&myDisplay, 4, 22, 123, 22);

    u8g2_SetFont(&myDisplay, u8g2_font_6x12_tf);
    u8g2_SetDrawColor(&myDisplay, 0);
    u8g2_DrawButtonUTF8(&myDisplay, 0, 127, 4, U8G2_BTN_BW0, 0, 0, page_name);

    // // Індикатор активного ряду
    // uint8_t rowIndicator = (currentActiveRow == ROW_1) ? 1 : 2;
    // switch (rowIndicator)
    // {
    // case 1:
    //     Paint_DrawRectangle(0, 24, 127, 75, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    //     break;
    // case 2:
    //     Paint_DrawRectangle(0, 75, 127, 127, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    //     break;
    // }
    u8g2_SendBuffer(&myDisplay);

    DrawParamBlocks();
}

void DrawEffectBlock(uint8_t slot)
{
    u8g2_ClearDisplay(&myDisplay);
    u8g2_SetDrawColor(&myDisplay, 1);

    EffectName selected = currentPreset.effectSlots[slot].selectedEffect;
    if (selected != EFFECT_NONE)
    {
        u8g2_SetFont(&myDisplay, u8g2_font_6x12_tf);
        u8g2_SetDrawColor(&myDisplay, 0);
        u8g2_DrawButtonUTF8(&myDisplay, 0, 127, 4, U8G2_BTN_BW0, 0, 0, effectLabels[selected]);
        u8g2_SetFont(&myDisplay, u8g2_font_6x12_tf);
        u8g2_SetDrawColor(&myDisplay, 0);
        u8g2_DrawButtonUTF8(&myDisplay, 0, 127, 4, U8G2_BTN_BW0, 0, 0, currentPreset.effectSlots[slot].isActive ? "On" : "Off");
    }
    else
    {
        Paint_TextCentered(" - ", 0, EFFECT_BLOCK_WIDTH, 0, Font12, WHITE, BLACK);
        Paint_TextCentered(" - ", 0, EFFECT_BLOCK_WIDTH, 16, Font12, WHITE, BLACK);
    }
    Paint_NumCentered(paramManager.GetNormalised(EFFECT_SLOT_DRYWET[slot]) * 100, 0, EFFECT_BLOCK_WIDTH, 32, 0, Font12, WHITE, BLACK);

    OLED_Part_Transmit_DMA(&effect_block_data[slot],
                           BLOCK_FX_X_START[slot],
                           BLOCK_FX_Y_START[slot],
                           BLOCK_FX_X_END[slot],
                           BLOCK_FX_Y_END[slot]);
}

void DrawEffectsPage()
{
    AssignParamsForPage(FX_PAGE);
    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_SetScale(16);
    Paint_Clear(BLACK);
    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_TextCentered(page_name, 0, 127, 4, Font12, WHITE, BLACK);

    Paint_TextCentered("Fx1", 0, 63, 40, Font12, WHITE, BLACK);
    Paint_TextCentered("Fx2", 64, 127, 40, Font12, WHITE, BLACK);
    Paint_DrawLine(56, 46, 72, 46, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(69, 43, 72, 46, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(69, 49, 72, 46, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    OLED_Transmit_DMA(&bg_black_data);

    for (size_t i = 0; i < 2; i++)
    {
        DrawEffectBlock(i);
    }
}

void SelectEffectPage(uint8_t slot)
{
    EffectName effect_to_show = currentPreset.effectSlots[slot].selectedEffect;
    MenuPage page = MenuPage::EMPTY;
    switch (effect_to_show)
    {
    case EFFECT_OVERDRIVE:
        page = MenuPage::OVERDRIVE_PAGE;
        break;
    case EFFECT_CHORUS:
        page = MenuPage::CHORUS_PAGE;
        break;
    case EFFECT_COMPRESSOR:
        page = MenuPage::COMPRESSOR_PAGE;
        break;
    case EFFECT_REVERB:
        page = MenuPage::REVERB_PAGE;
        break;
    case EFFECT_NONE:
        page = MenuPage::EMPTY;
        break;
    default:
        break;
    }
    SetPage(page);
}

void DrawModMatrixBlock(uint8_t blockIndex)
{
    Paint_NewImage(mod_matrix_block_data[blockIndex].data, MOD_MATRIX_BLOCK_WIDTH, MOD_MATRIX_BLOCK_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_TextCentered(currentPreset.modMtx[blockIndex].GetModSourceLabel(), 32, 64, 1, Font12, WHITE, BLACK);
    Paint_NumCentered(currentPreset.modMtx[blockIndex].GetModAmount() * 100, 64, 96, 1, 0, Font12, WHITE, BLACK);
    Paint_TextCentered(currentPreset.modMtx[blockIndex].GetModTargetLabel(), 96, 128, 1, Font12, WHITE, BLACK);
    if (blockIndex == selModBlockIndex)
    {
        uint8_t arrow_y = 8;
        Paint_DrawLine(8, arrow_y, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(20, arrow_y - 3, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(20, arrow_y + 3, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    }

    OLED_Part_Transmit_DMA(&mod_matrix_block_data[blockIndex],
                           BLOCK_MOD_MATRIX_X_START,
                           BLOCK_MOD_MATRIX_Y_START[blockIndex],
                           BLOCK_MOD_MATRIX_X_END,
                           BLOCK_MOD_MATRIX_Y_END[blockIndex]);
}

void DrawModMatrixBlocks()
{
    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        DrawModMatrixBlock(i);
    }
}

void DrawModMatrixPage()
{
    AssignParamsForPage(MOD_MATRIX_PAGE);
    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);
    
    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_TextCentered(page_name, 0, 127, 4, Font12, WHITE, BLACK);

    OLED_Transmit_DMA(&bg_black_data);

    DrawModMatrixBlocks(); 
}

void DrawWaveformImage(int waveform)
{

    switch (waveform)
    {
    case 0: // SIN
        Paint_DrawBitMapBlock(sin_wave, 32, 16, 0, 28);
        break;
    case 1: // TRI
        Paint_DrawBitMapBlock(tri_wave, 32, 16, 0, 28);
        break;
    case 2: // SAW
        Paint_DrawBitMapBlock(saw_wave, 32, 16, 0, 28);
        break;
    case 3: // SQR
        Paint_DrawBitMapBlock(sqr_wave, 32, 16, 0, 28);
        break;
    case 4: // NOISE
        Paint_DrawBitMapBlock(noise_wave, 32, 16, 0, 28);
        break;
    default:
        break;
    }
}

void DrawSettingsBlock(uint8_t blockIndex)
{
    Paint_NewImage(settings_block_data[blockIndex].data, SETTINGS_BLOCK_WIDTH, SETTINGS_BLOCK_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_TextCentered(paramManager.GetLabel(SETTINGS_PARAMS[blockIndex]), 32, 96, 1, Font12, WHITE, BLACK);
    if (paramManager.GetUnit(SETTINGS_PARAMS[blockIndex]) == ParamUnit::BOOL)
    {
        Paint_TextCentered(paramManager.GetBool(SETTINGS_PARAMS[blockIndex]) ? "On" : "Off", 96, 128, 1, Font12, WHITE, BLACK);
    }
    else
    {
        Paint_NumCentered((paramManager.GetValue(SETTINGS_PARAMS[blockIndex]) * 100), 96, 128, 1, 0, Font12, WHITE, BLACK);
    }

    if (blockIndex == selSettingsBlockIndex)
    {
        uint8_t arrow_y = 8;
        Paint_DrawLine(8, arrow_y, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(20, arrow_y - 3, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(20, arrow_y + 3, 24, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    }

    OLED_Part_Transmit_DMA(&settings_block_data[blockIndex],
                           BLOCK_SETTINGS_X_START,
                           BLOCK_SETTINGS_Y_START[blockIndex],
                           BLOCK_SETTINGS_X_END,
                           BLOCK_SETTINGS_Y_END[blockIndex]);
    
}

void DrawSettingsPage()
{
    AssignParamsForPage(SETTINGS_PAGE);
    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_TextCentered(page_name, 0, 127, 4, Font12, WHITE, BLACK);

    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_TextCentered(page_name, 0, 127, 4, Font12, WHITE, BLACK);

    OLED_Transmit_DMA(&bg_black_data);

    for (size_t i = 0; i < SETTINGS_BLOCKS_NUM; i++)
    {
        DrawSettingsBlock(i);
    }
}

void DrawStoreBlock()
{
    Paint_NewImage(store_block_data.data, STORE_BLOCK_WIDTH, STORE_BLOCK_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);
    
    Paint_DrawRectangle(1, 2, 96, 96, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_TextCentered("Store", 0, 96, 20, Font12, WHITE, BLACK);
    Paint_TextCentered("preset to", 0, 96, 36, Font12, WHITE, BLACK);
    Paint_NumCentered(currentPreset.number, 0, 96, 60, 0, Font16, WHITE, BLACK);

    OLED_Part_Transmit_DMA(&store_block_data,
                           BLOCK_STORE_X_START,
                           BLOCK_STORE_Y_START,
                           BLOCK_STORE_X_END,
                           BLOCK_STORE_Y_END);
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