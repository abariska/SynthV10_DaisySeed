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

    const uint8_t cursorX[4] = {6, 41, 72, 104};
    const uint8_t xStart[4]  = {0, 32, 64, 96};
    const uint8_t xEnd[4]    = {32, 64, 96, 127};
    const uint8_t yLabel = 32;
    const uint8_t yValue = 50;

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

            char valStr[16];
            sprintf(valStr, "%.2f", *param_unit.target_param);

            display.SetCursor(cursorX[i], yLabel);
            display.WriteString(param_unit.label, Font_6x8, true);
            
            display.SetCursor(cursorX[i], yValue);
            DisplayCentered(valStr, xStart[i], xEnd[i], yValue, Font_6x8, true);
        }
    }
}

inline void DrawMainLines()
{
    for (size_t raw = 0; raw < DISPLAY_HEIGHT; raw++)
    {
        for (size_t column = 0; column < DISPLAY_WIDTH; column += 2)
        {
            if (raw == 20)
                for (size_t i = 0; i < 127; i += 2)
                    display.DrawPixel(i, raw, true);
            else if (raw > 24 && raw % 2 == 0)
            {
                display.DrawPixel(32, raw, true);
                display.DrawPixel(65, raw, true);   
                display.DrawPixel(98, raw, true);
            }
            
        }
    }
} 

inline void DrawMainMenu()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];

    char cpu_load_value[PARAM_VALUE_LENGTH];
    float cpu_avg_load = cpu_load.GetAvgCpuLoad();

    DrawMainLines();
    
    display.SetCursor(5, 5);
    // safe_sprintf(prog_num, PROGRAM_NUMBER_LENGTH, "%03d", 1);
    sprintf(prog_num, "%03d", 1);
    display.WriteString(prog_num, Font_7x10, true);

    display.SetCursor(35, 5);
    // safe_sprintf(prog_name, PROGRAM_NAME_LENGTH, "Program");
    sprintf(prog_name, "Program");
    display.WriteString(prog_name, Font_7x10, true);

    display.SetCursor(100, 5);
    sprintf(cpu_load_value, "%.1f%%", cpu_avg_load * 100);
    display.WriteString(cpu_load_value, Font_7x10, true);

    UpdateParamsWithEncoders();

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
            InitParam(OSC_WAVEFORM_1, &params.voice.osc[0].waveform, "Wav", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_PITCH_1, &params.voice.osc[0].pitch, "Sem", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_DETUNE_1, &params.voice.osc[0].detune, "Det", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_AMP_1, &params.voice.osc[0].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;      
        case OSCILLATOR_2_PAGE:
            slots[0].assignedParam = OSC_WAVEFORM_2;
            slots[1].assignedParam = OSC_PITCH_2;
            slots[2].assignedParam = OSC_DETUNE_2;
            slots[3].assignedParam = OSC_AMP_2;
            InitParam(OSC_WAVEFORM_2, &params.voice.osc[1].waveform, "Wav", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_PITCH_2, &params.voice.osc[1].pitch, "Sem", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_DETUNE_2, &params.voice.osc[1].detune, "Det", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_AMP_2, &params.voice.osc[1].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;  
        case OSCILLATOR_3_PAGE:
            slots[0].assignedParam = OSC_WAVEFORM_3;
            slots[1].assignedParam = OSC_PITCH_3;
            slots[2].assignedParam = OSC_DETUNE_3;
            slots[3].assignedParam = OSC_AMP_3;
            InitParam(OSC_WAVEFORM_3, &params.voice.osc[2].waveform, "Wav", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_PITCH_3, &params.voice.osc[2].pitch, "Sem", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_DETUNE_3, &params.voice.osc[2].detune, "Det", 0.0f, 1.0f, 0.01f);
            InitParam(OSC_AMP_3, &params.voice.osc[2].amp, "Amp", 0.0f, 1.0f, 0.01f);
            break;  
        case AMPLIFIER_PAGE:
            slots[0].assignedParam = ADSR_ATTACK;
            slots[1].assignedParam = ADSR_DECAY;
            slots[2].assignedParam = ADSR_SUSTAIN;
            slots[3].assignedParam = ADSR_RELEASE;
            InitParam(ADSR_ATTACK, &params.voice.adsr.attack, "Att", 0.0f, 10.0f, 0.01f);
            InitParam(ADSR_DECAY, &params.voice.adsr.decay, "Dec", 0.0f, 10.0f, 0.01f);
            InitParam(ADSR_SUSTAIN, &params.voice.adsr.sustain, "Sus", 0.0f, 1.0f, 0.01f);
            InitParam(ADSR_RELEASE, &params.voice.adsr.release, "Rel", 0.0f, 10.0f, 0.01f);
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
            InitParam(LFO_WAVEFORM, &params.lfo.waveform, "Wav", 0.0f, 1.0f, 0.01f);
            InitParam(LFO_FREQ, &params.lfo.freq, "Frq", 0.0f, 1.0f, 0.01f);
            InitParam(LFO_DEPTH, &params.lfo.depth, "Amp", 0.0f, 1.0f, 0.01f);
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
    display.SetCursor(5, 15);
    DisplayCentered(page_name, 0, 127, 15, Font_7x10, true);
    slots[0].assignedParam = p0;
    slots[1].assignedParam = p1;
    slots[2].assignedParam = p2;
    slots[3].assignedParam = p3;
    UpdateParamsWithEncoders();
}

void DrawEffectsMenu() {
    // Horizontal line under header
    display.DrawLine(0, 20, 127, 20, true);

    display.SetCursor(5, 15);
    display.WriteString("Effects", Font_7x10, true);
    
    // Vertical lines for columns
    display.DrawLine(15, 20, 15, 63, true);
    display.DrawLine(90, 20, 90, 63, true);
    
    // Horizontal line between rows
    display.DrawLine(0, 40, 127, 40, true);
    
    // Data for first unit
    display.SetCursor(5, 30);
    display.WriteString("1", Font_7x10, true);

    // Data for first unit
    display.SetCursor(5, 50);
    display.WriteString("2", Font_7x10, true);

    for (size_t i = 0; i < 2; i++)
    {
        EffectName selected = effectSlot[i].selectedEffect;
        const int y = (i == 0) ? 30 : 50;

        if (selected != EFFECT_NONE)
        {
            display.SetCursor(25, y);
            display.WriteString(effectLabels[selected], Font_7x10, true);
            display.SetCursor(100, y);
            display.WriteString(effectSlot[i].isActive ? "On" : "Off", Font_7x10, true);
        }
        else
        {
            display.SetCursor(25, y);
            display.WriteString(" - ", Font_7x10, true);
            display.SetCursor(100, y);
            display.WriteString(" - ", Font_7x10, true);
        }
    }
}

void DrawMenu() {
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
            break;  

        case OSCILLATOR_2_PAGE:

            DrawParamPage("Oscillator 2", 
            OSC_WAVEFORM_2, OSC_PITCH_2, OSC_DETUNE_2, OSC_AMP_2);
            break;

        case OSCILLATOR_3_PAGE:

            DrawParamPage("Oscillator 3", 
            OSC_WAVEFORM_3, OSC_PITCH_3, OSC_DETUNE_3, OSC_AMP_3);
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
            EFFECT_OVERDRIVE_DRIVE, NONE, NONE, NONE);
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
            EFFECT_REVERB_DRYWET, EFFECT_REVERB_FBK, EFFECT_REVERB_LPFREQ, NONE);
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
#endif