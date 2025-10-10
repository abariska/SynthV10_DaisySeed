
#include "display.h"
#include "menu.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "effects.h"
#include "parameters.h"
#include "voice.h"

extern Preset currentPreset;
extern float scope_data[128];
extern int scope_data_index;
extern bool scope_data_ready;

const UWORD INTRO_PAGE_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD BG_BLACK_SIZE = (((FULL_PAGE_WIDTH % 2 == 0) ? (FULL_PAGE_WIDTH / 2) : (FULL_PAGE_WIDTH / 2 + 1)) * FULL_PAGE_HEIGHT);
const UWORD PARAM_BLOCK_SIZE = (((PARAM_BLOCK_WIDTH % 2 == 0) ? (PARAM_BLOCK_WIDTH / 2) : (PARAM_BLOCK_WIDTH / 2 + 1)) * PARAM_BLOCK_HEIGHT);
const UWORD WAVE_BUFFER_SIZE = (((WAVE_BUFFER_WIDTH % 2 == 0) ? (WAVE_BUFFER_WIDTH / 2) : (WAVE_BUFFER_WIDTH / 2 + 1)) * WAVE_BUFFER_HEIGHT);
const UWORD OSC_ON_BLOCK_SIZE = (((OSC_ON_BLOCK_WIDTH % 2 == 0) ? (OSC_ON_BLOCK_WIDTH / 2) : (OSC_ON_BLOCK_WIDTH / 2 + 1)) * OSC_ON_BLOCK_HEIGHT);
const UWORD EFFECT_BLOCK_SIZE = (((EFFECT_BLOCK_WIDTH % 2 == 0) ? (EFFECT_BLOCK_WIDTH / 2) : (EFFECT_BLOCK_WIDTH / 2 + 1)) * EFFECT_BLOCK_HEIGHT);
const UWORD CPU_LOAD_BLOCK_SIZE = (((CPU_LOAD_BLOCK_WIDTH % 2 == 0) ? (CPU_LOAD_BLOCK_WIDTH / 2) : (CPU_LOAD_BLOCK_WIDTH / 2 + 1)) * CPU_LOAD_BLOCK_HEIGHT);
const UWORD PRESET_NAME_BLOCK_SIZE = (((PRESET_NAME_BLOCK_WIDTH % 2 == 0) ? (PRESET_NAME_BLOCK_WIDTH / 2) : (PRESET_NAME_BLOCK_WIDTH / 2 + 1)) * PRESET_NAME_BLOCK_HEIGHT);
const UWORD PRESET_NUM_BLOCK_SIZE = (((PRESET_NUM_BLOCK_WIDTH % 2 == 0) ? (PRESET_NUM_BLOCK_WIDTH / 2) : (PRESET_NUM_BLOCK_WIDTH / 2 + 1)) * PRESET_NUM_BLOCK_HEIGHT);
const UWORD MOD_MATRIX_BLOCK_SIZE = (((MOD_MATRIX_BLOCK_WIDTH % 2 == 0) ? (MOD_MATRIX_BLOCK_WIDTH / 2) : (MOD_MATRIX_BLOCK_WIDTH / 2 + 1)) * MOD_MATRIX_BLOCK_HEIGHT);
const UWORD SCOPE_BLOCK_SIZE = (((SCOPE_BLOCK_WIDTH % 2 == 0) ? (SCOPE_BLOCK_WIDTH / 2) : (SCOPE_BLOCK_WIDTH / 2 + 1)) * SCOPE_BLOCK_HEIGHT);
const UWORD SETTINGS_BLOCK_SIZE = (((SETTINGS_BLOCK_WIDTH % 2 == 0) ? (SETTINGS_BLOCK_WIDTH / 2) : (SETTINGS_BLOCK_WIDTH / 2 + 1)) * SETTINGS_BLOCK_HEIGHT);
const UWORD STORE_BLOCK_SIZE = (((STORE_BLOCK_WIDTH % 2 == 0) ? (STORE_BLOCK_WIDTH / 2) : (STORE_BLOCK_WIDTH / 2 + 1)) * STORE_BLOCK_HEIGHT);
const UWORD VOICES_BLOCK_SIZE = (((VOICES_BLOCK_WIDTH % 2 == 0) ? (VOICES_BLOCK_WIDTH / 2) : (VOICES_BLOCK_WIDTH / 2 + 1)) * VOICES_BLOCK_HEIGHT);

UBYTE DSY_SDRAM_BSS intro_page[INTRO_PAGE_SIZE];
UBYTE DSY_SDRAM_BSS bg_black[BG_BLACK_SIZE];
UBYTE DSY_SDRAM_BSS param_block[NUM_PARAM_BLOCKS][PARAM_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS wave_buffer[WAVE_BUFFER_SIZE];
UBYTE DSY_SDRAM_BSS osc_on_block[OSC_ON_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS effect_block[NUM_FX_SLOTS][EFFECT_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS cpu_load_block[CPU_LOAD_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS preset_name_block[PRESET_NAME_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS preset_num_block[PRESET_NUM_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS mod_matrix_block[MOD_MATRIX_BLOCKS_NUM][MOD_MATRIX_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS scope_block[SCOPE_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS settings_block[SETTINGS_BLOCKS_NUM][SETTINGS_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS store_block[STORE_BLOCK_SIZE];
UBYTE DSY_SDRAM_BSS voices_block[VOICES_BLOCK_SIZE];
ImageData intro_page_data;
ImageData bg_black_data;
ImageData param_block_data[NUM_PARAM_BLOCKS];
ImageData wave_buffer_data;
ImageData osc_on_block_data;
ImageData effect_block_data[NUM_FX_SLOTS];
ImageData cpu_load_block_data;
ImageData preset_name_block_data;
ImageData preset_num_block_data;
ImageData mod_matrix_block_data[MOD_MATRIX_BLOCKS_NUM];
ImageData scope_block_data;
ImageData settings_block_data[SETTINGS_BLOCKS_NUM];
ImageData store_block_data;
ImageData voices_block_data;
MenuPage currentPage = MAIN_PAGE;

bool scope_draw = false;

void InitImages()
{

    memset(intro_page, 0, INTRO_PAGE_SIZE);
    memset(bg_black, 0, BG_BLACK_SIZE);
    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        memset(param_block[i], 0, PARAM_BLOCK_SIZE);
    }
    memset(wave_buffer, 0, WAVE_BUFFER_SIZE);
    memset(osc_on_block, 0, OSC_ON_BLOCK_SIZE);
    for (size_t i = 0; i < NUM_FX_SLOTS; i++)
    {
        memset(effect_block[i], 0, EFFECT_BLOCK_SIZE);
    }
    memset(effect_block, 0, EFFECT_BLOCK_SIZE);
    memset(cpu_load_block, 0, CPU_LOAD_BLOCK_SIZE);
    memset(preset_name_block, 0, PRESET_NAME_BLOCK_SIZE);
    memset(preset_num_block, 0, PRESET_NUM_BLOCK_SIZE);
    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        memset(mod_matrix_block[i], 0, MOD_MATRIX_BLOCK_SIZE);
    }
    memset(mod_matrix_block, 0, MOD_MATRIX_BLOCK_SIZE);
    memset(scope_block, 0, SCOPE_BLOCK_SIZE);
    memset(settings_block, 0, SETTINGS_BLOCK_SIZE);
    memset(store_block, 0, STORE_BLOCK_SIZE);
    memset(voices_block, 0, VOICES_BLOCK_SIZE);
    intro_page_data = {intro_page, INTRO_PAGE_SIZE};
    bg_black_data = {bg_black, BG_BLACK_SIZE};
    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        param_block_data[i] = {param_block[i], PARAM_BLOCK_SIZE};
    }
    wave_buffer_data = {wave_buffer, WAVE_BUFFER_SIZE};
    osc_on_block_data = {osc_on_block, OSC_ON_BLOCK_SIZE};
    for (size_t i = 0; i < NUM_FX_SLOTS; i++)
    {
        effect_block_data[i] = {effect_block[i], EFFECT_BLOCK_SIZE};
    }
    cpu_load_block_data = {cpu_load_block, CPU_LOAD_BLOCK_SIZE};
    preset_name_block_data = {preset_name_block, PRESET_NAME_BLOCK_SIZE};
    preset_num_block_data = {preset_num_block, PRESET_NUM_BLOCK_SIZE};
    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        mod_matrix_block_data[i] = {mod_matrix_block[i], MOD_MATRIX_BLOCK_SIZE};
    }
    scope_block_data = {scope_block, SCOPE_BLOCK_SIZE};
    for (size_t i = 0; i < SETTINGS_BLOCKS_NUM; i++)
    {
        settings_block_data[i] = {settings_block[i], SETTINGS_BLOCK_SIZE};
    }
    store_block_data = {store_block, STORE_BLOCK_SIZE};
    voices_block_data = {voices_block, VOICES_BLOCK_SIZE};
    System::Delay(10);
}

void DrawIntroPage()
{
    Paint_NewImage(intro_page_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_TextCentered("must B", 0, FULL_PAGE_WIDTH, 50, Font16, WHITE, BLACK);
    Paint_TextCentered("by abariska", 64, FULL_PAGE_WIDTH, 112, Font8, WHITE, BLACK);

    OLED_Transmit_DMA(&intro_page_data);
    System::Delay(1000);
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
        Paint_NewImage(scope_block_data.data, SCOPE_BLOCK_WIDTH, SCOPE_BLOCK_HEIGHT, 0, BLACK);
        Paint_Clear(BLACK);
        // uint8_t voice_num = 0;
        // for (size_t i = 0; i < VOICE_NUM; i++)
        // {
        //     voice_num += voiceState[i].active ? 1 : 0;
        // }
        float max_val = 0.0f;
        for (int i = 0; i < 128; i++)
        {
            float abs_val = fabs(scope_data[i]);
            if (abs_val > max_val) max_val = abs_val;
        }
        
        // Запобігаємо діленню на нуль
        if (max_val < 0.001f) max_val = 0.001f;
        
        // float scale = (SCOPE_BLOCK_HEIGHT * 0.5f) / max_val;  // 80% висоти екрану
        int center_y = SCOPE_BLOCK_HEIGHT / 2;
        
        for (int i = 0; i < SCOPE_BLOCK_WIDTH - 1; i++)
        {
            int data_index = (i * 128) / SCOPE_BLOCK_WIDTH;  // Інтерполяція
            int y = center_y - (int)(scope_data[data_index] * 40);
            
            // Обмежуємо координати
            if (y < 0) y = 0;
            if (y >= SCOPE_BLOCK_HEIGHT) y = SCOPE_BLOCK_HEIGHT - 1;
            
            Paint_DrawPoint(i, y, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT);
        }
        
        scope_data_ready = false;

        OLED_Part_Transmit_DMA(&scope_block_data,
                            BLOCK_SCOPE_X_START,
                            BLOCK_SCOPE_Y_START,
                            BLOCK_SCOPE_X_END,
                            BLOCK_SCOPE_Y_END);
            time_end = time1;
    }
    

}

void DrawMainPage()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    // char prog_name[PROGRAM_NAME_LENGTH];

    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_DrawLine(5, 18, 123, 18, 0x03, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(5, 20, 123, 20, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    sprintf(prog_num, "%03d", currentPreset.number);
    Paint_TextCentered(prog_num, 0, 127, 0, Font16, WHITE, BLACK);

    // sprintf(prog_name, "%s", currentPreset.name);
    // Paint_TextCentered(prog_name, 0, 127, 16, Font16, WHITE, BLACK);

    OLED_Transmit_DMA(&bg_black_data);
    DrawMainBlocks();
    DrawScope();
}

void DrawParamPage(MenuPage page)
{

    AssignParamsForPage(page);

    Paint_NewImage(bg_black_data.data, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_DrawLine(4, 22, 123, 22, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_TextCentered(page_name, 0, 127, 4, Font12, WHITE, BLACK);

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
    OLED_Transmit_DMA(&bg_black_data);

    DrawParamBlocks();
}

void DrawEffectBlock(uint8_t slot)
{
    Paint_NewImage(effect_block_data[slot].data, EFFECT_BLOCK_WIDTH, EFFECT_BLOCK_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    EffectName selected = currentPreset.effectSlots[slot].selectedEffect;
    if (selected != EFFECT_NONE)
    {
        Paint_TextCentered(effectLabels[selected], 0, EFFECT_BLOCK_WIDTH, 0, Font12, WHITE, BLACK);
        Paint_TextCentered(currentPreset.effectSlots[slot].isActive ? "On" : "Off", 0, EFFECT_BLOCK_WIDTH, 16, Font12, WHITE, BLACK);
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

    Paint_TextCentered(currentPreset.modMtx[blockIndex].GetModSourceLabel(), 32, 64, 0, Font12, WHITE, BLACK);
    Paint_NumCentered(currentPreset.modMtx[blockIndex].GetModAmount() * 100, 64, 96, 0, 0, Font12, WHITE, BLACK);
    Paint_TextCentered(currentPreset.modMtx[blockIndex].GetModTargetLabel(), 96, 128, 0, Font12, WHITE, BLACK);
    if (blockIndex == selModBlockIndex)
    {
        uint8_t arrow_y = 7;
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

    Paint_TextCentered(paramManager.GetLabel(SETTINGS_PARAMS[blockIndex]), 32, 96, 0, Font12, WHITE, BLACK);
    if (paramManager.GetType(SETTINGS_PARAMS[blockIndex]) == ParamType::DISCRETE)
    {
        Paint_TextCentered(paramManager.GetBool(SETTINGS_PARAMS[blockIndex]) ? "On" : "Off", 96, 128, 0, Font12, WHITE, BLACK);
    }
    else
    {
        Paint_NumCentered((paramManager.GetValue(SETTINGS_PARAMS[blockIndex]) * 100), 96, 128, 0, 0, Font12, WHITE, BLACK);
    }

    if (blockIndex == selSettingsBlockIndex)
    {
        uint8_t arrow_y = 7;
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