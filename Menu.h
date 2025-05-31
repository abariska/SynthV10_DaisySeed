#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "Display.h"
#include "Voice.h"
#include "Parameters.h"
#include "Effects.h"
#include "Encoder_mcp.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"
#include "OLED_1.5_Daisy_Seed/ImageData.h"
#include "OLED_1.5_Daisy_Seed/GUI_Paint.h"
#include "OLED_1.5_Daisy_Seed/OLED_Driver.h"
#include "OLED_1.5_Daisy_Seed/DEV_Config.h"
#include "Button.h"
#include "MidiHandler.h"
#include "GUI_Paint.h"

using namespace daisy;

extern CpuLoadMeter cpu_load;
extern Enc_mcp encoder_1;
extern Enc_mcp encoder_2; 
extern Enc_mcp encoder_3;
extern Enc_mcp encoder_4;
extern SynthParams params;
extern int encoderIncs[4];
bool isParamEditMode[4] = {false, false, false, false};
char page_name[16] = "";

const uint8_t yBlockLabel = 10;
const uint8_t yBlockValue = 30;

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

MenuPage currentPage = EMPTY;

enum ParamUnitName {
    OSC_WAVEFORM_1,
    OSC_PITCH_1,
    OSC_DETUNE_1,
    OSC_AMP_1,
    OSC_WAVEFORM_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_WAVEFORM_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    LFO_WAVEFORM,
    LFO_FREQ,
    LFO_DEPTH,
    EFFECT_OVERDRIVE_DRIVE,
    EFFECT_CHORUS_FREQ,
    EFFECT_CHORUS_DEPTH,
    EFFECT_CHORUS_FBK,
    EFFECT_CHORUS_PAN,
    EFFECT_COMPRESSOR_ATTACK,
    EFFECT_COMPRESSOR_RELEASE,
    EFFECT_COMPRESSOR_THRESHOLD,
    EFFECT_COMPRESSOR_RATIO,
    EFFECT_REVERB_DRYWET,
    EFFECT_REVERB_FBK,
    EFFECT_REVERB_LPFREQ,
    NONE
};

struct ParamUnitData{
    float* target_param;
    const char* label;
    float min;
    float max;
    float sensitivity;
    ValueType valueType;
} allParams[ParamUnitName::NONE + 1];

struct ParamSlot {
    ParamUnitName assignedParam;
    bool need_update;
} slots[NUM_PARAM_BLOCKS];

void EncoderChangeEffect();
void DrawPages();
void DrawEffectsMenu();
void InitParam(ParamUnitName param, uint8_t slotIndex);

void AssignParamsForPage(MenuPage page);

void DrawWaveformImage(int waveform){
    
    switch (waveform)
    {
    case 0: // 0
        Paint_DrawBitMapBlock(tri_wave, 32, 16, 0, 28); 
        break;
    case 1: // 1
        Paint_DrawBitMapBlock(saw_wave, 32, 16, 0, 28);
        break;
    case 2: // 2
        Paint_DrawBitMapBlock(sqr_wave, 32, 16, 0, 28);
        break;
    case 3: // 3
        break;
    default:
        break;
    }
}

void InitOneParamBlock(int blockIndex, float value, const char* label, uint16_t textColor = WHITE, uint16_t bgColor = BLACK){

    Paint_NewImage(param_block[blockIndex], PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, bgColor); 
    Paint_SetScale(16);
    Paint_Clear(bgColor);
    
    Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, Font12, textColor, bgColor);
    switch (allParams[slots[blockIndex].assignedParam].valueType) {
            case REGULAR:
                Paint_NumCentered(value, 0, PARAM_BLOCK_WIDTH, yBlockValue, 0, Font12, textColor, bgColor);
                break;
            case X100:
                Paint_NumCentered(value * 100, 0, PARAM_BLOCK_WIDTH, yBlockValue, 0, Font12, textColor, bgColor);
                break;
            case WAVEFORM:
                DrawWaveformImage((Waves)value);
                break;
            }
    OLED_1in5_Display_Part(param_block[blockIndex], BLOCK_X_START[blockIndex], 
        BLOCK_TOP_LINE_Y, BLOCK_X_END[blockIndex], BLOCK_BOTTOM_LINE_Y);
}

void InitParamBlocks(){
    if (currentPage == FX_PAGE) {
        return;
    }
    
    for (size_t i = 0; i < 4; i++) {
        if (slots[i].assignedParam == NONE) {
            continue;
        }
        InitOneParamBlock(i, *allParams[slots[i].assignedParam].target_param, allParams[slots[i].assignedParam].label);   
    }
}

void EditBlockParam(int blockIndex) {
    static uint32_t lastBlinkTime = 0;
    static bool blinkState = false;
    
    int currentTime = System::GetNow();
    
    if (currentTime - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        lastBlinkTime = currentTime;
    }
    
    uint16_t textColor = blinkState ? BLACK : WHITE;
    uint16_t bgColor = blinkState ? 0x01 : BLACK;
    
    Paint_SelectImage(param_block[blockIndex]); 
    Paint_SetScale(16);
    Paint_Clear(bgColor);
    
    int value = (int)slots[blockIndex].assignedParam;
        
    value += encoderIncs[blockIndex];
        
    if (value > (int)NONE) {
        value = (int)NONE;
    } else if (value < 0) {
        value = 0;
    }
        
    slots[blockIndex].assignedParam = (ParamUnitName)value;
    InitParam((ParamUnitName)value, blockIndex); 
    InitOneParamBlock(blockIndex, *allParams[slots[blockIndex].assignedParam].target_param, 
        allParams[slots[blockIndex].assignedParam].label, textColor, bgColor);
}

void UpdateParamsWithEncoders() {

    static uint8_t isCurrentEditSlot = 0;

    if (currentPage == FX_PAGE) {
        EncoderChangeEffect();
    }
    else {
        if (currentPage == MAIN_PAGE) {
            for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
                if (isParamEditMode[i]) {
                    if (isCurrentEditSlot != i) {
                        isCurrentEditSlot = i;
                        for (size_t j = 0; j < NUM_PARAM_BLOCKS; j++) {
                            if (j != i) {
                                isParamEditMode[j] = false;
                                InitOneParamBlock(j, *allParams[slots[j].assignedParam].target_param, 
                                    allParams[slots[j].assignedParam].label, WHITE, BLACK);
                            }
                        }
                        
                    }
                    EditBlockParam(i);
                }
            }
        }
    
        for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
            if (encoderIncs[i] != 0 && !isParamEditMode[i]) {
                slots[i].need_update = true;
            }
        }
        
        for (size_t i = 0; i < 4; i++) {
            if (!slots[i].need_update) continue;

            float* target_param = allParams[slots[i].assignedParam].target_param;
            if (target_param == nullptr) continue;
   
            if (slots[i].need_update) {
                float value = *target_param;
                
                value += encoderIncs[i] * allParams[slots[i].assignedParam].sensitivity;

                if (value > allParams[slots[i].assignedParam].max) {
                    value = allParams[slots[i].assignedParam].max;
                } else if (value < allParams[slots[i].assignedParam].min) {
                    value = allParams[slots[i].assignedParam].min;
                }
                
                *target_param = value;
                InitOneParamBlock(i, value, allParams[slots[i].assignedParam].label);
            }
            slots[i].need_update = false;
        }
    }
}

void DrawMainPage()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];

    DrawStaticPage(BLACK);
    
    sprintf(prog_num, "%03d", 1);
    Paint_TextCentered(prog_num, 0, 127, 0, Font16, WHITE, BLACK);

    sprintf(prog_name, "Program");
    Paint_TextCentered(prog_name, 0, 127, 16, Font16, WHITE, BLACK);
    InitParamBlocks();
}

void SetPageName(const char* name) {
    strcpy(page_name, name);
}

void SetPage(MenuPage newPage) {
    if (currentPage == newPage) {
        return;
    }
    currentPage = newPage;    
    AssignParamsForPage(newPage); 
    DrawPages();
    InitParamBlocks();
}

void AssignParamsForPage(MenuPage page) {

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].assignedParam = NONE;
    }
    
    switch (page) {
        case MAIN_PAGE:
            SetPageName("");
            InitParam(FILTER_CUTOFF, 0);
            InitParam(FILTER_RESONANCE, 1);
            InitParam(ADSR_ATTACK, 2);
            InitParam(ADSR_DECAY, 3);
            break;
        case OSCILLATOR_1_PAGE:
            SetPageName("Oscillator 1");
            InitParam(OSC_WAVEFORM_1, 0);
            InitParam(OSC_PITCH_1, 1);
            InitParam(OSC_DETUNE_1, 2);
            InitParam(OSC_AMP_1, 3);
            break;      
        case OSCILLATOR_2_PAGE:
            SetPageName("Oscillator 2");
            InitParam(OSC_WAVEFORM_2, 0);
            InitParam(OSC_PITCH_2, 1);
            InitParam(OSC_DETUNE_2, 2);
            InitParam(OSC_AMP_2, 3);
            break;      
        case OSCILLATOR_3_PAGE:
            SetPageName("Oscillator 3");
            InitParam(OSC_WAVEFORM_3, 0);
            InitParam(OSC_PITCH_3, 1);
            InitParam(OSC_DETUNE_3, 2);
            InitParam(OSC_AMP_3, 3);
            break;  
        case AMPLIFIER_PAGE:
            SetPageName("Amplifier");
            InitParam(ADSR_ATTACK, 0);
            InitParam(ADSR_DECAY, 1);
            InitParam(ADSR_SUSTAIN, 2);
            InitParam(ADSR_RELEASE, 3);
            break;  
        case FILTER_PAGE:
            SetPageName("Filter");
            InitParam(FILTER_CUTOFF, 0);
            InitParam(FILTER_RESONANCE, 1);
            break;  
        case LFO_PAGE:
            SetPageName("LFO");
            InitParam(LFO_WAVEFORM, 0);
            InitParam(LFO_FREQ, 1);
            InitParam(LFO_DEPTH, 2);
          break;
        case FX_PAGE:
            SetPageName("Effects");
            break;
        case OVERDRIVE_PAGE:
            SetPageName("Overdrive");
            InitParam(EFFECT_OVERDRIVE_DRIVE, 0);
            break;
        case CHORUS_PAGE: 
            SetPageName("Chorus");
            InitParam(EFFECT_CHORUS_FREQ, 0);
            InitParam(EFFECT_CHORUS_DEPTH, 1);
            InitParam(EFFECT_CHORUS_FBK, 2);
            InitParam(EFFECT_CHORUS_PAN, 3);
            break;
        case COMPRESSOR_PAGE:
            SetPageName("Compressor");
            InitParam(EFFECT_COMPRESSOR_ATTACK, 0);
            InitParam(EFFECT_COMPRESSOR_RELEASE, 1);
            InitParam(EFFECT_COMPRESSOR_THRESHOLD, 2);
            InitParam(EFFECT_COMPRESSOR_RATIO, 3);
            break;
        case REVERB_PAGE:
            SetPageName("Reverb");
            InitParam(EFFECT_REVERB_FBK, 0);
            InitParam(EFFECT_REVERB_LPFREQ, 1);
            InitParam(EFFECT_REVERB_DRYWET, 2);
            break;
        default: 
            SetPageName(" - ");
            InitParam(NONE, 0);
            InitParam(NONE, 1);
            InitParam(NONE, 2);
            InitParam(NONE, 3);
            break;
    }
}

void DrawPages() {
    // Paint_SelectImage(background_black);
    // Paint_SetScale(16);
    // Paint_Clear(BLACK);
    if (currentPage == MAIN_PAGE)
    {
        DrawMainPage();
    }

    else if (currentPage == OSCILLATOR_1_PAGE 
        || currentPage == OSCILLATOR_2_PAGE 
        || currentPage == OSCILLATOR_3_PAGE)
    {
        DrawStaticParamPage(BLACK);
        Paint_DrawString_EN(110, 0,params.voice.osc[0].active ? "On" : "Off", &Font8, WHITE, BLACK);
    }
    
    else if (currentPage == FX_PAGE)
    {
        DrawFXLines();
    }
    
    else {
        DrawStaticParamPage(BLACK);
    }

    Paint_TextCentered(page_name, 0, 127, 0, Font12, WHITE, BLACK);
    OLED_1in5_Display(background_black);
}

void DrawEffectsMenu() {
    Paint_NewImage(background_black, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    Paint_SetScale(16); 
    Paint_Clear(BLACK); 
    // Paint_TextCentered("Effects", 0, 127, 0, Font12, WHITE, BLACK);
    // Horizontal line under header
    for (size_t raw = 0; raw < FULL_PAGE_HEIGHT; raw++)
    {
        for (size_t column = 0; column < FULL_PAGE_WIDTH; column += 3)
        {
            if (raw == 20 || raw == 40)
                for (size_t i = 0; i < 127; i += 3)
                    Paint_DrawPoint(i, raw, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT);
            if (raw > 20 && raw % 3 == 0)
            {
                Paint_DrawPoint(64, raw, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT); 
            }
        }
    }
    
    Paint_TextCentered("1", 0, 63, 20, Font12, WHITE, BLACK);
    Paint_TextCentered("2", 64, 127, 20, Font12, WHITE, BLACK);

    for (size_t i = 0; i < 2; i++)
    {
        EffectName selected = effectSlot[i].selectedEffect;
        const int x1 = (i == 0) ? 0 : 64;
        const int x2 = (i == 0) ? 64 : 127;
        const int y1 = 64;
        const int y2 = 80;
        if (selected != EFFECT_NONE)
        {
            Paint_TextCentered(effectLabels[selected], x1, x2, y1, Font12, WHITE, BLACK);
            Paint_TextCentered(effectSlot[i].isActive ? "On" : "Off", x1, x2, y2, Font12, WHITE, BLACK);
        }
        else
        {
            Paint_TextCentered(" - ", x1, x2, y1, Font12, WHITE, BLACK);
            Paint_TextCentered(" - ", x1, x2, y2, Font12, WHITE, BLACK);
        }
    }
}

void EncoderChangeEffect() {
    if (currentPage == MenuPage::FX_PAGE) {
        if (encoderIncs[0] != 0) {
            int newEffect = static_cast<int>(effectSlot[0].selectedEffect) + encoderIncs[0];
            if (newEffect < 0) newEffect = 0;
            if (newEffect >= 4) newEffect = 4;
            effectSlot[0].selectedEffect = static_cast<EffectName>(newEffect);
        }
        if (encoderIncs[3] != 0) {
            int newEffect = static_cast<int>(effectSlot[1].selectedEffect) + encoderIncs[3];
            if (newEffect < 0) newEffect = 0;
            if (newEffect >= 4) newEffect = 4;
            effectSlot[1].selectedEffect = static_cast<EffectName>(newEffect);
        }
    }
}

void CpuUsageDisplay(){
    
    if (currentPage == MAIN_PAGE) {
        Paint_SelectImage(cpu_load_block);
        Paint_SetScale(16);
        Paint_Clear(BLACK);
        float cpu_avg_load = cpu_load.GetAvgCpuLoad();
        Paint_NumCentered(cpu_avg_load, 0, 15, 0, 1, Font8, WHITE, BLACK);
        OLED_1in5_Display_Part(cpu_load_block, 111, 0, 127, 15);
    }
}

// Array of parameter initialization data
const ParamUnitData paramInitTable[] = {
    { &params.voice.osc[0].waveform, "Wav", 0, 3, 1, WAVEFORM },
    { &params.voice.osc[0].pitch, "Sem", -36, 36, 1, X100 },
    { &params.voice.osc[0].detune, "Det", -0.5, 0.5, 0.01, X100 },
    { &params.voice.osc[0].amp, "Amp", 0, 1, 0.01, X100 },
    { &params.voice.osc[1].waveform, "Wav", 0, 3, 1, WAVEFORM },    
    { &params.voice.osc[1].pitch, "Sem", -36, 36, 1, X100 },
    { &params.voice.osc[1].detune, "Det", -0.5, 0.5, 0.01, X100 },
    { &params.voice.osc[1].amp, "Amp", 0, 1, 0.01, X100 },
    { &params.voice.osc[2].waveform, "Wav", 0, 3, 1, WAVEFORM },    
    { &params.voice.osc[2].pitch, "Sem", -36, 36, 1, X100 },
    { &params.voice.osc[2].detune, "Det", -0.5, 0.5, 0.01, X100 },
    { &params.voice.osc[2].amp, "Amp", 0, 1, 0.01, X100 },
    { &params.voice.filter.cutoff, "Cut", 50, 15000, 1, REGULAR },
    { &params.voice.filter.resonance, "Res", 0, 1, 0.01, X100 },
    { &params.voice.adsr.attack, "Atk", 0, 1, 0.01, X100 },
    { &params.voice.adsr.decay, "Dec", 0, 1, 0.01, X100 },
    { &params.voice.adsr.sustain, "Sus", 0, 1, 0.01, X100 },
    { &params.voice.adsr.release, "Rel", 0, 1, 0.01, X100 },
    { &params.lfo.freq, "Freq", 0, 1, 0.01, REGULAR },
    { &params.lfo.depth, "Dpth", 0, 1, 0.01, X100 },
    { &params.lfo.waveform, "Wav", 0, 3, 1, WAVEFORM },
    { &params.overdriveParams.drive, "Drv", 0, 1, 0.01, X100 },
    { &params.chorusParams.freq, "Freq", 0, 1, 0.01, REGULAR },
    { &params.chorusParams.depth, "Dpth", 0, 1, 0.01, X100 },
    { &params.chorusParams.delay, "Dly", 0, 1, 0.01, X100 },
    { &params.chorusParams.feedback, "Fbk", 0, 1, 0.01, X100 },
    { &params.compressorParams.attack, "Atk", 0, 1, 0.01, X100 },
    { &params.compressorParams.release, "Rel", 0, 1, 0.01, X100 },
    { &params.compressorParams.threshold, "Thr", 0, 1, 0.01, X100 },
    { &params.compressorParams.ratio, "Ratio", 0, 1, 0.01, X100 },
    // { &params.compressorParams.makeup, "Makeup", 0, 1, 0.01, X100 },
    { &params.reverbParams.dryWet, "Dry", 0, 1, 0.01, X100 },
    { &params.reverbParams.feedback, "Fbk", 0, 1, 0.01, X100 },
    { &params.reverbParams.lpFreq, "LPF", 0, 1, 0.01, X100 }
};

void InitParam(ParamUnitName param, uint8_t slotIndex) {

    slots[slotIndex].assignedParam = param;

    if (param < NONE && param < sizeof(paramInitTable)/sizeof(paramInitTable[0])) {
        allParams[param].target_param = paramInitTable[param].target_param;
        allParams[param].label = paramInitTable[param].label;
        allParams[param].min = paramInitTable[param].min;
        allParams[param].max = paramInitTable[param].max;
        allParams[param].sensitivity = paramInitTable[param].sensitivity;
        allParams[param].valueType = paramInitTable[param].valueType;
    } else {
        // Handle NONE or unknown parameter
        allParams[param].target_param = nullptr;
        allParams[param].label = " - ";
        allParams[param].min = 0;
        allParams[param].max = 0;
        allParams[param].sensitivity = 0;
        allParams[param].valueType = REGULAR;
    }
}
    


#endif