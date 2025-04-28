#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "DisplayOLED.h"
#include "Voice.h"
#include "Parameters.h"
#include "Effects.h"
#include "Encoder_mcp.h"

#define PROGRAM_NAME_LENGTH 12
#define PROGRAM_NUMBER_LENGTH 4
#define PARAM_NAME_LENGTH 8
#define PARAM_VALUE_LENGTH 8

using namespace daisy;

extern MyOledDisplay display;
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
    EFFECT_OVERDRIVE_MAKEUP,
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
} slots[4 + 1];

inline void DisplayCentered(const char* text, uint8_t x1, uint8_t x2, uint8_t y, FontDef font, bool color);
    
void UpdateParamsWithEncoders() {

    for (size_t i = 0; i < 4; i++) {
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
        }
    }
}

inline void DrawMainLines()
{
    for (size_t raw = 0; raw < DISPLAY_HEIGHT; raw++)
    {
        for (size_t column = 0; column < DISPLAY_WIDTH; column += 2)
        {
            if (raw == 15)
                for (size_t i = 0; i < 127; i += 3)
                    display.DrawPixel(i, raw, true);
            else if (raw > 15 && raw % 3 == 0)
            {
                display.DrawPixel(30, raw, true);
                display.DrawPixel(64, raw, true);   
                display.DrawPixel(98, raw, true);
            }
            
        }
    }
} 

void DrawParameters(ParamUnitName p0, ParamUnitName p1, ParamUnitName p2, ParamUnitName p3) {
    const uint8_t xStart[4]  = {0, 32, 66, 102};
    const uint8_t xEnd[4]    = {30, 65, 99, 127};
    const uint8_t yLabel = 24;
    const uint8_t yValue = 48;

    slots[0].assignedParam = p0;
    slots[1].assignedParam = p1;
    slots[2].assignedParam = p2;
    slots[3].assignedParam = p3;
    // UpdateParamsWithEncoders();

    for (size_t i = 0; i < 4; i++){
        char valStr[16];
        if (slots[i].assignedParam == NONE) {
            continue;
        }
        if (slots[i].assignedParam == OSC_WAVEFORM_1 || 
            slots[i].assignedParam == OSC_PITCH_1 || 
            slots[i].assignedParam == OSC_WAVEFORM_2 || 
            slots[i].assignedParam == OSC_PITCH_2 || 
            slots[i].assignedParam == OSC_WAVEFORM_3 ||
            slots[i].assignedParam == OSC_PITCH_3 ||
            slots[i].assignedParam == FILTER_CUTOFF ||
            slots[i].assignedParam == LFO_WAVEFORM) {
            sprintf(valStr, "%d", (int)*allParams[slots[i].assignedParam].target_param);
        } else {
            sprintf(valStr, "%d", (int)(*allParams[slots[i].assignedParam].target_param * 100));
        }
        DisplayCentered(allParams[slots[i].assignedParam].label, xStart[i], xEnd[i], yLabel, Font_6x8, true);
        DisplayCentered(valStr, xStart[i], xEnd[i], yValue, Font_6x8, true);
    }
}

inline void DrawMainMenu()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];

    char cpu_load_value[PARAM_VALUE_LENGTH];
    float cpu_avg_load = cpu_load.GetAvgCpuLoad();

    DrawMainLines();
    
    display.SetCursor(5, 0);
    // safe_sprintf(prog_num, PROGRAM_NUMBER_LENGTH, "%03d", 1);
    sprintf(prog_num, "%03d", 1);
    display.WriteString(prog_num, Font_7x10, true);

    display.SetCursor(35, 0);
    // safe_sprintf(prog_name, PROGRAM_NAME_LENGTH, "Program");
    sprintf(prog_name, "Program");
    display.WriteString(prog_name, Font_7x10, true);

    display.SetCursor(100, 0);
    sprintf(cpu_load_value, "%.1f%%", cpu_avg_load * 100);
    display.WriteString(cpu_load_value, Font_7x10, true);

    DrawParameters(FILTER_CUTOFF, FILTER_RESONANCE, ADSR_ATTACK, ADSR_DECAY);
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
            slots[1].assignedParam = EFFECT_OVERDRIVE_MAKEUP;
            InitParam(EFFECT_OVERDRIVE_DRIVE, &params.overdriveParams.drive, "Drive", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_OVERDRIVE_MAKEUP, &params.overdriveParams.makeup, "Vol", -0.5f, 1.0f, 0.01f);
            break;
        case CHORUS_PAGE:
            slots[0].assignedParam = EFFECT_CHORUS_FREQ;
            slots[1].assignedParam = EFFECT_CHORUS_DEPTH;
            slots[2].assignedParam = EFFECT_CHORUS_FBK;
            slots[3].assignedParam = EFFECT_CHORUS_PAN;
            InitParam(EFFECT_CHORUS_FREQ, &params.chorusParams.freq, "Freq", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_CHORUS_DEPTH, &params.chorusParams.depth, "Amp", 0.0f, 1.0f, 0.01f);
            InitParam(EFFECT_CHORUS_FBK, &params.chorusParams.feedback, "Fbk", 0.0f, 1.0f, 0.01f);
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
            InitParam(EFFECT_COMPRESSOR_RATIO, &params.compressorParams.ratio, "Rat", 0.0f, 1.0f, 0.01f);
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

void DrawParamPage(const char* page_name, ParamUnitName p0, ParamUnitName p1, ParamUnitName p2, ParamUnitName p3) {
    DrawMainLines();
    DisplayCentered(page_name, 0, 127, 0, Font_7x10, true);
    DrawParameters(p0, p1, p2, p3);
}

void BoolDisplay(uint8_t x1, uint8_t x2, uint8_t y, FontDef font, bool color, bool inValue) {
    bool value = inValue;
    const char* text = value ? "On" : "Off";
    uint8_t textLength = 0;
    
    for (const char* c = text; *c != '\0'; c++) {
        textLength++;
    }
    
    // Determine character width based on font
    uint8_t charWidth = 0;
    if (font.data == Font_6x8.data) {
        charWidth = 6;  // For font 6x8
    } else if (font.data == Font_7x10.data) {
        charWidth = 7;  // For font 7x10
    } else {
        charWidth = 8;  // For other fonts
    }
    
    uint8_t textWidth = textLength * charWidth;
    
    // Calculate x-coordinate for centering
    uint8_t middleX = x1 + (x2 - x1) / 2;
    uint8_t startX = middleX - (textWidth / 2);
    
    // Correction if text goes beyond boundaries
    if (startX < x1) startX = x1;
    if (startX + textWidth > x2) startX = x2 - textWidth;
    
    // Output text
    display.SetCursor(startX, y);
    display.WriteString(text, font, color);
}

void DrawEffectsMenu() {

    DrawMainLines();
    DisplayCentered("Effects", 0, 127, 0, Font_7x10, true);
    display.SetCursor(110, 0);
    display.WriteString(isFXActive ? "On" : "Off", Font_6x8, true);

    const uint8_t xStart[4]  = {0, 32, 66, 102};
    const uint8_t xEnd[4]    = {30, 65, 99, 127};
    const uint8_t yLabel = 24;
    const uint8_t yValue = 48;

    DisplayCentered("Drv", xStart[0], xEnd[0], yLabel, Font_7x10, true);
    BoolDisplay(xStart[0], xEnd[0], yValue, Font_6x8, true, params.overdriveParams.isActive);
    DisplayCentered("Cmp", xStart[1], xEnd[1], yLabel, Font_7x10, true);
    BoolDisplay(xStart[1], xEnd[1], yValue, Font_6x8, true, params.compressorParams.isActive);
    DisplayCentered("Chr", xStart[2], xEnd[2], yLabel, Font_7x10, true);
    BoolDisplay(xStart[2], xEnd[2], yValue, Font_6x8, true, params.chorusParams.isActive);
    DisplayCentered("Rvb", xStart[3], xEnd[3], yLabel, Font_7x10, true);
    BoolDisplay(xStart[3], xEnd[3], yValue, Font_6x8, true, params.reverbParams.isActive);
}

void DrawPages() {
    // Clear display before showing new menu
    display.Fill(false);

    AssignParamsForPage(currentPage);
    
    // Display different pages depending on selected menu
    switch (currentPage) {

        case MAIN_PAGE:
            DrawMainMenu();
            break;
            
        case OSCILLATOR_1_PAGE:

            DrawParamPage("Oscillator 1", 
            OSC_WAVEFORM_1, OSC_PITCH_1, OSC_DETUNE_1, OSC_AMP_1);
            display.SetCursor(110, 0);
            display.WriteString(params.voice.osc[0].active ? "On" : "Off", Font_6x8, true);
            break;  

        case OSCILLATOR_2_PAGE:

            DrawParamPage("Oscillator 2", 
            OSC_WAVEFORM_2, OSC_PITCH_2, OSC_DETUNE_2, OSC_AMP_2);
            display.SetCursor(110, 0);
            display.WriteString(params.voice.osc[1].active ? "On" : "Off", Font_6x8, true);
            break;

        case OSCILLATOR_3_PAGE:

            DrawParamPage("Oscillator 3", 
            OSC_WAVEFORM_3, OSC_PITCH_3, OSC_DETUNE_3, OSC_AMP_3);
            display.SetCursor(110, 0);
            display.WriteString(params.voice.osc[2].active ? "On" : "Off", Font_6x8, true);
            break;  
            
        case AMPLIFIER_PAGE:

            DrawParamPage("Amplifier", 
            ADSR_ATTACK, ADSR_DECAY, ADSR_SUSTAIN, ADSR_RELEASE);
            break;          

        case FILTER_PAGE:

            DrawParamPage("Filter", 
            FILTER_CUTOFF, FILTER_RESONANCE, NONE, NONE);
            break;  

        case LFO_PAGE:

            DrawParamPage("LFO", 
            LFO_WAVEFORM, LFO_FREQ, LFO_DEPTH, NONE);
            break;  
            
        case FX_PAGE:
            DrawEffectsMenu();
            break;

        case OVERDRIVE_PAGE:
            DrawParamPage("Overdrive", 
            EFFECT_OVERDRIVE_DRIVE, EFFECT_OVERDRIVE_MAKEUP, NONE, NONE);
            break;

        case CHORUS_PAGE:
            DrawParamPage("Chorus", 
            EFFECT_CHORUS_FREQ, EFFECT_CHORUS_DEPTH, EFFECT_CHORUS_FBK, EFFECT_CHORUS_PAN);
            break;

        case COMPRESSOR_PAGE:
            DrawParamPage("Compressor", 
            EFFECT_COMPRESSOR_ATTACK, EFFECT_COMPRESSOR_RELEASE, EFFECT_COMPRESSOR_THRESHOLD, EFFECT_COMPRESSOR_RATIO);
            break;

        case REVERB_PAGE:
            DrawParamPage("Reverb", 
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
    display.Update();
}

inline void DisplayCentered(const char* text, uint8_t x1, uint8_t x2, uint8_t y, FontDef font, bool color) {
    // Calculate text length (number of characters)
    int textLength = 0;
    for (const char* c = text; *c != '\0'; c++) {
        textLength++;
    }
    
    // Determine character width based on font
    int charWidth = 0;
    if (font.data == Font_6x8.data) {
        charWidth = 6;  // For font 6x8
    } else if (font.data == Font_7x10.data) {
        charWidth = 7;  // For font 7x10
    } else {
        charWidth = 8;  // For other fonts
    }
    
    int textWidth = textLength * charWidth;
    
    // Calculate x-coordinate for centering
    uint8_t middleX = x1 + (x2 - x1) / 2;
    uint8_t startX = middleX - (textWidth / 2);
    
    // Correction if text goes beyond boundaries
    if (startX < x1) startX = x1;
    if (startX + textWidth > x2) startX = x2 - textWidth;
    
    // Output text
    display.SetCursor(startX, y);
    display.WriteString(text, font, color);
}

void SetPage(MenuPage newPage) {
    currentPage = newPage;
    
    AssignParamsForPage(newPage); 
}

void EditParameterPage(MenuPage page) {
    currentPage = page;
    AssignParamsForPage(page);
}


#endif