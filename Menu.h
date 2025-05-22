#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "Display.h"
#include "Voice.h"
#include "Parameters.h"
#include "Effects.h"
#include "Encoder_mcp.h"
#include "OLED_1.5_Daisy_Seed/fonts.h"

using namespace daisy;

extern CpuLoadMeter cpu_load;
extern Enc_mcp encoder_1;
extern Enc_mcp encoder_2; 
extern Enc_mcp encoder_3;
extern Enc_mcp encoder_4;
extern SynthParams params;
extern int encoderIncs[4];

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
};

MenuPage currentPage = MAIN_PAGE;

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
} allParams[ParamUnitName::NONE + 1];

struct ParamSlot {
    ParamUnitName assignedParam;
    bool need_update;
} slots[NUM_PARAM_BLOCKS];

void EncoderChangeEffect();
void DrawPages();

void InitParamBlocks(){
    for (size_t i = 0; i < 4; i++) {
        const char* label = allParams[slots[i].assignedParam].label;
        const float value = *allParams[slots[i].assignedParam].target_param * 100;
        

        Paint_NewImage(param_block[i], PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, BLACK); 
        Paint_SetScale(16);
        Paint_Clear(BLACK);
        
        Paint_TextCentered(label, 0, 32, 10, Font12, WHITE, BLACK);
        Paint_NumCentered((int)value, 0, 32, 30, 0, Font12, WHITE, BLACK);
        OLED_1in5_Display_Part(param_block[i], BLOCK_X_START[i], BLOCK_TOP_LINE_Y, BLOCK_X_END[i], BLOCK_BOTTOM_LINE_Y);
    }
}

void CheckBlockParamForUpdate(){

    static float current_param_value[NUM_PARAM_BLOCKS];
    static float new_param_value[NUM_PARAM_BLOCKS];
    static bool initialized = false;
    
    if (!initialized) {
        for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
            if (allParams[slots[i].assignedParam].target_param != NULL) {
                new_param_value[i] = *allParams[slots[i].assignedParam].target_param;
            }
            if (allParams[slots[i].assignedParam].target_param != NULL) {
                current_param_value[i] = *allParams[slots[i].assignedParam].target_param;
            }
        }
        initialized = true;
    }

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        if (new_param_value[i] != current_param_value[i]) {
            current_param_value[i] = new_param_value[i];
            slots[i].need_update = true;
        }
    }
}

void UpdateParamsWithEncoders() {

    if (currentPage == MenuPage::FX_PAGE) {
        EncoderChangeEffect();
        return;
    }
    
    const uint8_t xStart[4]  = {0, 32, 66, 102};
    const uint8_t xEnd[4]    = {30, 65, 99, 127};
    const uint8_t yLabel = 24;
    const uint8_t yValue = 48;

    int param_value;

    for (size_t i = 0; i < 4; i++) {
        if (!slots[i].need_update) continue;

        ParamUnitName paramID = slots[i].assignedParam;
        if (paramID == NONE) continue; // слот пустий

        ParamUnitData& param_unit = allParams[paramID];     
        if (param_unit.target_param != nullptr) {

            *param_unit.target_param += encoderIncs[i] * param_unit.sensitivity;

            if (*param_unit.target_param > param_unit.max) {
                *param_unit.target_param = param_unit.max;
            } else if (*param_unit.target_param < param_unit.min) {
                *param_unit.target_param = param_unit.min;
            }
            param_value = (int)*param_unit.target_param;
            if (paramID == OSC_WAVEFORM_1 || 
                paramID == OSC_PITCH_1 || 
                paramID == OSC_WAVEFORM_2 || 
                paramID == OSC_PITCH_2 || 
                paramID == OSC_WAVEFORM_3 ||
                paramID == OSC_PITCH_3 ||
                paramID == FILTER_CUTOFF) {
                param_value = (int)*param_unit.target_param;
            } else {
                param_value = (int)(*param_unit.target_param * 100);
            }
            for (size_t i = 0; i < 4; i++) {
                Paint_NewImage(param_block[i], PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, BLACK); 
                Paint_SetScale(16);
                Paint_Clear(BLACK);

                Paint_TextCentered(param_unit.label, 0, 32, 10, Font12, WHITE, BLACK);
                Paint_NumCentered(param_value, 0, 32, 30, 0, Font12, WHITE, BLACK);
                
                OLED_1in5_Display_Part(param_block[i], BLOCK_X_START[i], BLOCK_TOP_LINE_Y, BLOCK_X_END[i], BLOCK_BOTTOM_LINE_Y);
            }
        }
        slots[i].need_update = false;
    }
}

inline void DrawMainPageLines()
{
    for (size_t raw = 0; raw < FULL_PAGE_HEIGHT; raw++)
    {
        for (size_t column = 0; column < FULL_PAGE_WIDTH; column += 2)
        {
            if (raw == 15)
                for (size_t i = 0; i < FULL_PAGE_WIDTH; i += 3)
                    Paint_DrawPoint(i, raw, 0x1, DOT_PIXEL_1X1, DOT_STYLE_DFT);
            else if (raw > 15 && raw % 3 == 0)
            {
                Paint_DrawPoint(30, raw, 0x1, DOT_PIXEL_1X1, DOT_STYLE_DFT);
                Paint_DrawPoint(64, raw, 0x1, DOT_PIXEL_1X1, DOT_STYLE_DFT);   
                Paint_DrawPoint(98, raw, 0x1, DOT_PIXEL_1X1, DOT_STYLE_DFT);
            }
            
        }
    }
} 

inline void DrawMainPage()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];

    float cpu_avg_load = cpu_load.GetAvgCpuLoad();

    DrawStaticPage(BLACK);
    
    sprintf(prog_num, "%03d", 1);
    Paint_TextCentered(prog_num, 0, 127, 0, Font16, WHITE, BLACK);

    sprintf(prog_name, "Program");
    Paint_TextCentered(prog_name, 0, 127, 16, Font16, WHITE, BLACK);

    Paint_NumCentered(cpu_avg_load * 100, 100, 127, 0, 1, Font8, WHITE, BLACK);

    OLED_1in5_Display(background_black);
}

void InitParam(int index, float* target, const char* label, float min, float max, float sensitivity) {
    allParams[index] = { target, label, min, max, sensitivity };
}

void AssignParamsForPage(MenuPage page) {
    switch (page) {
        case MAIN_PAGE:
            slots[0].assignedParam = FILTER_CUTOFF;
            slots[1].assignedParam = FILTER_RESONANCE;
            slots[2].assignedParam = ADSR_ATTACK;
            slots[3].assignedParam = ADSR_DECAY;
            InitParam(FILTER_CUTOFF, &params.voice.filter.cutoff, "Cut", 50.0f, 15000.0f, 10.0f);
            InitParam(FILTER_RESONANCE, &params.voice.filter.resonance, "Res", 0.0f, 1.0f, 0.01f);
            InitParam(ADSR_ATTACK, &params.voice.adsr.attack, "Att", 0.0f, 10.0f, 0.01f);
            InitParam(ADSR_DECAY, &params.voice.adsr.decay, "Dec", 0.0f, 10.0f, 0.01f);
            break;
        case OSCILLATOR_1_PAGE:
            slots[0].assignedParam = OSC_WAVEFORM_1;
            slots[1].assignedParam = OSC_PITCH_1;
            slots[2].assignedParam = OSC_DETUNE_1;
            slots[3].assignedParam = OSC_AMP_1;
            InitParam(OSC_WAVEFORM_1, &params.voice.osc[0].waveform, "Wav", 0.0f, 3.0f, 1.0f);
            InitParam(OSC_PITCH_1, &params.voice.osc[0].pitch, "Sem", -36.0f, 36.0f, 1.0f);
            InitParam(OSC_DETUNE_1, &params.voice.osc[0].detune, "Det", -0.5f, 0.5f, 0.01f);
            InitParam(OSC_AMP_1, &params.voice.osc[0].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;      
        case OSCILLATOR_2_PAGE:
            slots[0].assignedParam = OSC_WAVEFORM_2;
            slots[1].assignedParam = OSC_PITCH_2;
            slots[2].assignedParam = OSC_DETUNE_2;
            slots[3].assignedParam = OSC_AMP_2;
            InitParam(OSC_WAVEFORM_2, &params.voice.osc[1].waveform, "Wav", 0.0f, 3.0f, 1.0f);
            InitParam(OSC_PITCH_2, &params.voice.osc[1].pitch, "Sem", -36.0f, 36.0f, 1.0f);
            InitParam(OSC_DETUNE_2, &params.voice.osc[1].detune, "Det", -0.5f, 0.5f, 0.01f);
            InitParam(OSC_AMP_2, &params.voice.osc[1].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;      
        case OSCILLATOR_3_PAGE:
            slots[0].assignedParam = OSC_WAVEFORM_3;
            slots[1].assignedParam = OSC_PITCH_3;
            slots[2].assignedParam = OSC_DETUNE_3;
            slots[3].assignedParam = OSC_AMP_3;
            InitParam(OSC_WAVEFORM_3, &params.voice.osc[2].waveform, "Wav", 0.0f, 3.0f, 1.0f);
            InitParam(OSC_PITCH_3, &params.voice.osc[2].pitch, "Sem", -36.0f, 36.0f, 1.0f);
            InitParam(OSC_DETUNE_3, &params.voice.osc[2].detune, "Det", -0.5f, 0.5f, 0.01f);
            InitParam(OSC_AMP_3, &params.voice.osc[2].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;  
        case AMPLIFIER_PAGE:
            slots[0].assignedParam = ADSR_ATTACK;
            slots[1].assignedParam = ADSR_DECAY;
            slots[2].assignedParam = ADSR_SUSTAIN;
            slots[3].assignedParam = ADSR_RELEASE;
            InitParam(ADSR_ATTACK, &params.voice.adsr.attack, "Att", 0.0f, 1.0f, 0.01f);
            InitParam(ADSR_DECAY, &params.voice.adsr.decay, "Dec", 0.0f, 1.0f, 0.01f);
            InitParam(ADSR_SUSTAIN, &params.voice.adsr.sustain, "Sus", 0.0f, 1.0f, 0.01f);
            InitParam(ADSR_RELEASE, &params.voice.adsr.release, "Rel", 0.0f, 1.0f, 0.01f);
            break;  
        case FILTER_PAGE:
            slots[0].assignedParam = FILTER_CUTOFF;
            slots[1].assignedParam = FILTER_RESONANCE;
            InitParam(FILTER_CUTOFF, &params.voice.filter.cutoff, "Cut", 50.0f, 15000.0f, 10.0f);
            InitParam(FILTER_RESONANCE, &params.voice.filter.resonance, "Res", 0.0f, 1.0f, 0.01f);
            break;  
        case LFO_PAGE:
            slots[0].assignedParam = LFO_WAVEFORM;
            slots[1].assignedParam = LFO_FREQ;
            slots[2].assignedParam = LFO_DEPTH;
            InitParam(LFO_WAVEFORM, &params.lfo.waveform, "Wav", 0.0f, 5.0f, 1.0f);
            InitParam(LFO_FREQ, &params.lfo.freq, "Frq", 0.0f, 1.0f, 0.01f);
            InitParam(LFO_DEPTH, &params.lfo.depth, "Amp", 0.0f, 1.0f, 0.01f);
            break;
        case FX_PAGE:
            break;
        case OVERDRIVE_PAGE:
            slots[0].assignedParam = EFFECT_OVERDRIVE_DRIVE ;
            InitParam(EFFECT_OVERDRIVE_DRIVE, &params.overdriveParams.drive, "Drive", 0.0f, 1.0f, 0.01f);
            break;
        case CHORUS_PAGE:
            slots[0].assignedParam = EFFECT_CHORUS_FREQ;
            slots[1].assignedParam = EFFECT_CHORUS_DEPTH;
            slots[2].assignedParam = EFFECT_CHORUS_FBK;
            slots[3].assignedParam = EFFECT_CHORUS_PAN;
            InitParam(EFFECT_CHORUS_FREQ, &params.chorusParams.freq, "Freq", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_CHORUS_DEPTH, &params.chorusParams.depth, "Depth", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_CHORUS_FBK, &params.chorusParams.feedback, "FBK", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_CHORUS_PAN, &params.chorusParams.pan, "Pan", 0.0f, 1.0f, 0.01f);
            break;
        case COMPRESSOR_PAGE:
            slots[0].assignedParam = EFFECT_COMPRESSOR_ATTACK;
            slots[1].assignedParam = EFFECT_COMPRESSOR_RELEASE;
            slots[2].assignedParam = EFFECT_COMPRESSOR_THRESHOLD;
            slots[3].assignedParam = EFFECT_COMPRESSOR_RATIO;
            InitParam(EFFECT_COMPRESSOR_ATTACK, &params.compressorParams.attack, "Att", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_COMPRESSOR_RELEASE, &params.compressorParams.release, "Rel", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_COMPRESSOR_THRESHOLD, &params.compressorParams.threshold, "Thr", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_COMPRESSOR_RATIO, &params.compressorParams.ratio, "Ratio", 0.0f, 1.0f, 0.01f);
            break;
        case REVERB_PAGE:
            slots[0].assignedParam = EFFECT_REVERB_FBK;
            slots[1].assignedParam = EFFECT_REVERB_LPFREQ;
            slots[2].assignedParam = EFFECT_REVERB_DRYWET   ;
            InitParam(EFFECT_REVERB_FBK, &params.reverbParams.feedback, "Fbk", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_REVERB_LPFREQ, &params.reverbParams.lpFreq, "Lpf", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_REVERB_DRYWET, &params.reverbParams.dryWet, "Wet", 0.0f, 1.0f, 0.01f);
            break;
        default: 
            slots[0].assignedParam = NONE;
            slots[1].assignedParam = NONE;
            slots[2].assignedParam = NONE;
            slots[3].assignedParam = NONE;
            InitParam(NONE, nullptr, " - ", 0.0f, 1.0f, 0.01f);
            break;
    }
}

void SetParamPageContent(const char* page_name, ParamUnitName p0, ParamUnitName p1, ParamUnitName p2, ParamUnitName p3) {
    DrawStaticParamPage(BLACK);
    Paint_TextCentered(page_name, 0, 127, 0, Font12, WHITE, BLACK);
    slots[0].assignedParam = p0;
    slots[1].assignedParam = p1;
    slots[2].assignedParam = p2;
    slots[3].assignedParam = p3;
}

void DrawEffectsMenu() {

    Paint_TextCentered("Effects", 0, 127, 0, Font8, WHITE, BLACK);
    // Horizontal line under header
    for (size_t raw = 0; raw < FULL_PAGE_HEIGHT; raw++)
    {
        for (size_t column = 0; column < FULL_PAGE_WIDTH; column += 3)
        {
            if (raw == 15 || raw == 32)
                for (size_t i = 0; i < 127; i += 3)
                    Paint_DrawPoint(i, raw, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT);
            if (raw > 15 && raw % 3 == 0)
            {
                Paint_DrawPoint(64, raw, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT); 
            }
            
        }
    }
    
    Paint_TextCentered("1", 0, 63, 20, Font8, BLACK, WHITE);
    Paint_TextCentered("2", 64, 127, 20, Font8, BLACK, WHITE);

    for (size_t i = 0; i < 2; i++)
    {
        EffectName selected = effectSlot[i].selectedEffect;
        const int x1 = (i == 0) ? 0 : 64;
        const int x2 = (i == 0) ? 64 : 127;
        const int y1 = 35;
        const int y2 = 48;
        if (selected != EFFECT_NONE)
        {
            Paint_TextCentered(effectLabels[selected], x1, x2, 35, Font8, BLACK, WHITE);
            Paint_TextCentered(effectSlot[i].isActive ? "On" : "Off", x1, x2, 48, Font8, BLACK, WHITE);

        }
        else
        {
            Paint_TextCentered(" - ", x1, x2, y1, Font8, BLACK, WHITE);
            Paint_TextCentered(" - ", x1, x2, y2, Font8, BLACK, WHITE);
        }
    }
}

void DrawPages() {
    
    // Display different pages depending on selected menu
    switch (currentPage) {

        case MAIN_PAGE:
            DrawMainPage();
            break;
            
        case OSCILLATOR_1_PAGE:
            SetParamPageContent("Oscillator 1", 
            OSC_WAVEFORM_1, OSC_PITCH_1, OSC_DETUNE_1, OSC_AMP_1);
            Paint_DrawString_EN(110, 0,params.voice.osc[0].active ? "On" : "Off", &Font8, WHITE, BLACK);
            break;  

        case OSCILLATOR_2_PAGE:
            SetParamPageContent("Oscillator 2", 
            OSC_WAVEFORM_2, OSC_PITCH_2, OSC_DETUNE_2, OSC_AMP_2);
            Paint_DrawString_EN(110, 0,params.voice.osc[1].active ? "On" : "Off", &Font8, WHITE, BLACK);
            break;

        case OSCILLATOR_3_PAGE:
            SetParamPageContent("Oscillator 3", 
            OSC_WAVEFORM_3, OSC_PITCH_3, OSC_DETUNE_3, OSC_AMP_3);
            Paint_DrawString_EN(110, 0,params.voice.osc[2].active ? "On" : "Off", &Font8, WHITE, BLACK);
            break;  
            
        case AMPLIFIER_PAGE:
            SetParamPageContent("Amplifier", 
            ADSR_ATTACK, ADSR_DECAY, ADSR_SUSTAIN, ADSR_RELEASE);
            break;          

        case FILTER_PAGE:
            SetParamPageContent("Filter", 
            FILTER_CUTOFF, FILTER_RESONANCE, NONE, NONE);
            break;  

        case LFO_PAGE:
            SetParamPageContent("LFO", 
            LFO_WAVEFORM, LFO_FREQ, LFO_DEPTH, NONE);
            break;  
            
        case FX_PAGE:
            DrawEffectsMenu();
            break;

        case OVERDRIVE_PAGE:
            SetParamPageContent("Overdrive", 
            EFFECT_OVERDRIVE_DRIVE, NONE, NONE, NONE);
            break;

        case CHORUS_PAGE:
            SetParamPageContent("Chorus", 
            EFFECT_CHORUS_FREQ, EFFECT_CHORUS_DEPTH, EFFECT_CHORUS_FBK, EFFECT_CHORUS_PAN);
            break;

        case COMPRESSOR_PAGE:
            SetParamPageContent("Compressor", 
            EFFECT_COMPRESSOR_ATTACK, EFFECT_COMPRESSOR_RELEASE, EFFECT_COMPRESSOR_THRESHOLD, EFFECT_COMPRESSOR_RATIO);
            break;

        case REVERB_PAGE:
            SetParamPageContent("Reverb", 
            EFFECT_REVERB_FBK, EFFECT_REVERB_LPFREQ,EFFECT_REVERB_DRYWET, NONE);
            break;
            
        case MTX_PAGE:
            break;
            
        case SETTINGS_PAGE:
            break;
            
        case STORE_PAGE:
            break;
            
        case LOAD_PAGE  :
            break;
    }    
    OLED_1in5_Display(background_black);
    InitParamBlocks();
}

void SetPage(MenuPage newPage) {
    currentPage = newPage;    
    AssignParamsForPage(newPage); 
    DrawPages();
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


#endif