#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "DisplayOLED.h"
#include "Voice.h"
#include "Parameters.h"

#define PROGRAM_NAME_LENGTH 12
#define PROGRAM_NUMBER_LENGTH 4
#define PARAM_NAME_LENGTH 8
#define PARAM_VALUE_LENGTH 8

using namespace daisy;

extern MyOledDisplay display;
extern CpuLoadMeter cpu_load;
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

inline void DrawMainLines()
{
    for (int raw = 0; raw < DISPLAY_HEIGHT; raw++)
    {
        for (int column = 0; column < DISPLAY_WIDTH; column += 2)
        {
            if (raw == 20)
                display.DrawLine(0, 24, 127, 24, true);
            else if (raw > 24 && raw % 2 == 0)
            {
                display.DrawPixel(32, raw, true);
                display.DrawPixel(65, raw, true);   
                display.DrawPixel(98, raw, true);
            }
            
        }
    }
} 

inline void DisplayCentered(const char* text, uint8_t x1, uint8_t x2, uint8_t y, FontDef font, bool color) {
    // Обчислення довжини тексту (кількість символів)
    int textLength = 0;
    for (const char* c = text; *c != '\0'; c++) {
        textLength++;
    }
    
    // Визначаємо ширину символу в залежності від шрифту
    int charWidth = 0;
    if (font.data == Font_6x8.data) {
        charWidth = 6;  // Для шрифту 6x8
    } else if (font.data == Font_7x10.data) {
        charWidth = 7;  // Для шрифту 7x10
    } else {
        charWidth = 8;  // Для інших шрифтів
    }
    
    int textWidth = textLength * charWidth;
    
    // Обчислення x-координати для центрування
    uint8_t middleX = x1 + (x2 - x1) / 2;
    uint8_t startX = middleX - (textWidth / 2);
    
    // Корекція, якщо текст виходить за межі
    if (startX < x1) startX = x1;
    if (startX + textWidth > x2) startX = x2 - textWidth;
    
    // Вивід тексту
    display.SetCursor(startX, y);
    display.WriteString(text, font, color);
}

// Функція для безпечного форматування рядків
inline void safe_sprintf(char* buffer, size_t buffer_size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, buffer_size - 1, format, args);
    va_end(args);
    buffer[buffer_size - 1] = '\0'; // Забезпечуємо нуль-термінатор
}

inline void DrawMainMenu()
{
    char prog_num[PROGRAM_NUMBER_LENGTH];
    char prog_name[PROGRAM_NAME_LENGTH];
    char encoder_1_param[PARAM_NAME_LENGTH];
    char encoder_1_value[PARAM_VALUE_LENGTH];
    char encoder_2_param[PARAM_NAME_LENGTH];
    char encoder_2_value[PARAM_VALUE_LENGTH];
    char encoder_3_param[PARAM_NAME_LENGTH];
    char encoder_3_value[PARAM_VALUE_LENGTH];
    char encoder_4_param[PARAM_NAME_LENGTH];
    char encoder_4_value[PARAM_VALUE_LENGTH];

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

    // Відображення параметрів з структури params
    display.SetCursor(6, 32);
    // safe_sprintf(encoder_1_param, PARAM_NAME_LENGTH, "Cut");
    sprintf(encoder_1_param, "Cut");
    display.WriteString(encoder_1_param, Font_6x8, true);

    display.SetCursor(6, 50);
    // safe_sprintf(encoder_1_value, PARAM_VALUE_LENGTH, expected_format_1, params.voice.filter.cutoff);
    sprintf(encoder_1_value, "%d", static_cast<int>(params.voice.filter.cutoff));
    DisplayCentered(encoder_1_value, 0, 32, 50, Font_6x8, true);

    display.SetCursor(41, 32);
    // safe_sprintf(encoder_2_param, PARAM_NAME_LENGTH, "Res");
    sprintf(encoder_2_param, "Res");
    display.WriteString(encoder_2_param, Font_6x8, true);

    display.SetCursor(41, 50);
    // safe_sprintf(encoder_2_value, PARAM_VALUE_LENGTH, expected_format_2, params.voice.filter.resonance);
    sprintf(encoder_2_value, "%d", static_cast<int>(params.voice.filter.resonance * 100));
    DisplayCentered(encoder_2_value, 32, 64, 50, Font_6x8, true);

    display.SetCursor(72, 32);
    // safe_sprintf(encoder_3_param, PARAM_NAME_LENGTH, "Att");
    sprintf(encoder_3_param, "Att");
    display.WriteString(encoder_3_param, Font_6x8, true);

    display.SetCursor(72, 50);
    // safe_sprintf(encoder_3_value, PARAM_VALUE_LENGTH, expected_format_3, params.voice.adsr.attack);
    sprintf(encoder_3_value, "%d", static_cast<int>(params.voice.adsr.attack * 100));
    DisplayCentered(encoder_3_value, 64, 96, 50, Font_6x8, true);

    display.SetCursor(104, 32);
    // safe_sprintf(encoder_4_param, PARAM_NAME_LENGTH, "Dec");
    sprintf(encoder_4_param, "Dec");
    display.WriteString(encoder_4_param, Font_6x8, true);

    display.SetCursor(104, 50);
    // safe_sprintf(encoder_4_value, PARAM_VALUE_LENGTH, expected_format_4, params.voice.adsr.decay);
    sprintf(encoder_4_value, "%d", static_cast<int>(params.voice.adsr.decay * 100));
    DisplayCentered(encoder_4_value, 96, 127, 50, Font_6x8, true);
    
    display.Update();
}

inline void DrawMenusParameters(
    const char* menu_name, 
    const char* param_for_enc_1,
    const char* param_for_enc_2,
    const char* param_for_enc_3,
    const char* param_for_enc_4,
    int value_for_enc_1, 
    int value_for_enc_2, 
    int value_for_enc_3, 
    int value_for_enc_4,
    bool value_for_top_right_corner)
{
    char enc_1_value[PARAM_VALUE_LENGTH];
    char enc_2_value[PARAM_VALUE_LENGTH];
    char enc_3_value[PARAM_VALUE_LENGTH];
    char enc_4_value[PARAM_VALUE_LENGTH];
    char top_right_corner_value[PARAM_VALUE_LENGTH];

    display.SetCursor(6, 6);
    display.WriteString(menu_name, Font_7x10, true);

    if (currentPage == OSCILLATOR_1_PAGE || currentPage == OSCILLATOR_2_PAGE || currentPage == OSCILLATOR_3_PAGE) {
        display.SetCursor(104, 6);
        safe_sprintf(top_right_corner_value, PARAM_VALUE_LENGTH, "%s", value_for_top_right_corner ? "ON" : "OFF");
        display.WriteString(top_right_corner_value, Font_7x10, true);
    }
    display.SetCursor(6, 32);
    display.WriteString(param_for_enc_1, Font_6x8, true);

    display.SetCursor(6, 50);
    safe_sprintf(enc_1_value, PARAM_VALUE_LENGTH, "%d", value_for_enc_1);
    DisplayCentered(enc_1_value, 0, 32, 50, Font_6x8, true);

    display.SetCursor(41, 32);
    display.WriteString(param_for_enc_2, Font_6x8, true);

    display.SetCursor(41, 50);
    safe_sprintf(enc_2_value, PARAM_VALUE_LENGTH, "%d", value_for_enc_2);
    DisplayCentered(enc_2_value, 32, 64, 50, Font_6x8, true);

    display.SetCursor(72, 32);
    display.WriteString(param_for_enc_3, Font_6x8, true);

    display.SetCursor(72, 50);
    safe_sprintf(enc_3_value, PARAM_VALUE_LENGTH, "%d", value_for_enc_3);
    DisplayCentered(enc_3_value, 64, 96, 50, Font_6x8, true);

    display.SetCursor(104, 32);
    display.WriteString(param_for_enc_4, Font_6x8, true);

    display.SetCursor(104, 50);
    safe_sprintf(enc_4_value, PARAM_VALUE_LENGTH, "%d", value_for_enc_4);
    DisplayCentered(enc_4_value, 96, 127, 50, Font_6x8, true);
}


inline void DrawEffectsMenu() {
    // Горизонтальна лінія під заголовком
    display.DrawLine(0, 20, 127, 20, true);
    
    // Вертикальні лінії для стовпців
    display.DrawLine(15, 20, 15, 63, true);
    display.DrawLine(90, 20, 90, 63, true);
    
    // Горизонтальна лінія між рядками
    display.DrawLine(0, 40, 127, 40, true);

    display.SetCursor(5, 15);
    display.WriteString("Effects", Font_7x10, true);
    
    // Дані для першого юніта
    display.SetCursor(5, 30);
    display.WriteString("1", Font_7x10, true);
    
    
    if (params.effectUnits[0].activeEffect != EFFECT_NONE) {
        display.SetCursor(25, 30);
        display.WriteString(GetEffectName(params.effectUnits[0].activeEffect), Font_7x10, true);
        display.SetCursor(100, 30);
        display.WriteString(params.effectUnits[0].isActive ? "On" : "Off", Font_7x10, true);
    } else {
        display.SetCursor(25, 30);
        display.WriteString(" - ", Font_7x10, true);
        display.SetCursor(100, 30);
        display.WriteString(" - ", Font_7x10, true);
    }
    
    if (params.effectUnits[1].activeEffect != EFFECT_NONE) {
        display.SetCursor(5, 50);
        display.WriteString("2", Font_7x10, true);
        display.SetCursor(25, 50);
        display.WriteString(GetEffectName(params.effectUnits[1].activeEffect), Font_7x10, true);
        display.SetCursor(100, 50);
        display.WriteString(params.effectUnits[1].isActive ? "On" : "Off", Font_7x10, true);
    } else {
        display.SetCursor(5, 50);
        display.WriteString("2", Font_7x10, true);
        display.SetCursor(25, 50);
        display.WriteString(" - ", Font_7x10, true);
        display.SetCursor(100, 50);
        display.WriteString(" - ", Font_7x10, true);
    }
}

inline void DrawEffectUnit(int unit_number) {
    switch (params.effectUnits[unit_number].activeEffect)
    {
    case EFFECT_OVERDRIVE:
        DrawMenusParameters(
            "Overdrive",
            "Drive",
            " - ",
            " - ",
            " - ",
            params.effectUnits[unit_number].overdrive.drive,
            0,
            0,
            0,
            params.effectUnits[unit_number].overdrive.isActive
        );
        break;
    case EFFECT_CHORUS:
        DrawMenusParameters(
            "Chorus",
            "Freq",
            "Amp",
            "Fbck",
            "Pan",
            params.effectUnits[unit_number].chorus.lfoFreq,
            params.effectUnits[unit_number].chorus.lfoDepth,
            params.effectUnits[unit_number].chorus.feedback,
            params.effectUnits[unit_number].chorus.pan,
            params.effectUnits[unit_number].chorus.isActive
        );
        break;
    case EFFECT_COMPRESSOR:
        DrawMenusParameters(
            "Compressor",
            "Att",
            "Rel",
            "Thr",
            "Ratio",
            params.effectUnits[unit_number].compressor.attack,
            params.effectUnits[unit_number].compressor.release,
            params.effectUnits[unit_number].compressor.threshold,
            params.effectUnits[unit_number].compressor.ratio,
            params.effectUnits[unit_number].compressor.isActive
        );
        break;
    case EFFECT_REVERB:
        DrawMenusParameters(
            "Reverb",
            "Dry",
            "Fbck",
            "Lp",
            " - ",
            params.effectUnits[unit_number].reverb.dryWet,
            params.effectUnits[unit_number].reverb.feedback,
            params.effectUnits[unit_number].reverb.lpFreq,
            0,
            params.effectUnits[unit_number].reverb.isActive
        );
        break;
    case EFFECT_NONE:
        DrawMenusParameters(
            " - ",
            " - ",
            " - ",
            " - ",
            " - ",
            0,
            0,
            0,
            0,
            false
        );
        break;
    default:
        break;
    }
}

void DrawMenu() {
    // Очищаємо дисплей перед відображенням нового меню
    display.Fill(false);
    
    // Відображаємо різні сторінки залежно від вибраного меню
    switch (currentPage) {

        case MAIN_PAGE:
            DrawMainMenu();
            break;
            
        case OSCILLATOR_1_PAGE:
            DrawMainLines();
            DrawMenusParameters(
                "Oscillator 1",
                "Wav",
                "Oct",
                "Tun",
                "Amp",
                (params.voice.osc[0].waveform),
                (params.voice.osc[0].pitch),
                static_cast<int>(params.voice.osc[0].detune * 100),
                static_cast<int>(params.voice.osc[0].amp * 100),
                params.voice.osc[0].active
            );
            break;

        case OSCILLATOR_2_PAGE:

            DrawMainLines();        

            DrawMenusParameters(
                "Oscillator 2",
                "Wav",
                "Oct",
                "Tun",
                "Amp",
                (params.voice.osc[1].waveform),
                (params.voice.osc[1].pitch),
                static_cast<int>(params.voice.osc[1].detune * 100),
                static_cast<int>(params.voice.osc[1].amp * 100),
                (params.voice.osc[1].active)
            );
            break;

        case OSCILLATOR_3_PAGE:

            DrawMainLines();

            DrawMenusParameters(
                "Oscillator 3",
                "Wav",
                "Oct",
                "Tun",
                "Amp",
                (params.voice.osc[2].waveform),
                (params.voice.osc[2].pitch),
                static_cast<int>(params.voice.osc[2].detune * 100),
                static_cast<int>(params.voice.osc[2].amp * 100),
                params.voice.osc[2].active
            );
            break;
            
        case AMPLIFIER_PAGE:

            DrawMainLines();

            DrawMenusParameters(
                "Amplifier",
                "Att",
                "Dec",
                "Sus",
                "Rel",
                static_cast<int>(params.voice.adsr.attack * 100),
                static_cast<int>(params.voice.adsr.decay * 100),
                static_cast<int>(params.voice.adsr.sustain * 100),
                static_cast<int>(params.voice.adsr.release * 100),
                false
            );  
            break;

        case FILTER_PAGE:

            DrawMainLines();

            DrawMenusParameters(
                "Filter",
                "Cut",
                "Res",
                " - ",
                " - ",
                static_cast<int>(params.voice.filter.cutoff * 100),
                static_cast<int>(params.voice.filter.resonance * 100),
                0,
                0,
                false
            );
            break;  
        case LFO_PAGE:

            DrawMainLines();

            DrawMenusParameters(
                "LFO",
                "Wav",
                "Frq",
                "Amp",    
                " - ",
                (params.lfo.waveform),
                (params.lfo.freq),
                static_cast<int>(params.lfo.amp * 100),
                0,      
                false
            );  
            break;
            
        case FX_PAGE:
            DrawEffectsMenu();
            break;

        case OVERDRIVE_PAGE:
            DrawEffectUnit(0);
            break;

        case CHORUS_PAGE:
            DrawEffectUnit(1);
            break;

        case COMPRESSOR_PAGE:
            DrawEffectUnit(2);
            break;

        case REVERB_PAGE:
            DrawEffectUnit(3);
            break;
            
        case MTX_PAGE:
            // Модуляційна матриця
            display.SetCursor(5, 15);
            display.WriteString("Modulation", Font_7x10, true);
            
            display.SetCursor(5, 30);
            display.WriteString("Source -> Dest", Font_7x10, true);
            
            display.SetCursor(5, 45);
            display.WriteString("Amt: 0.50", Font_7x10, true);
            break;
            
        case SETTINGS_PAGE:
            // Налаштування
            display.SetCursor(5, 15);
            display.WriteString("Settings", Font_7x10, true);
            break;
            
        case STORE_PAGE:
            // Збереження програми
            display.SetCursor(5, 15);
            display.WriteString("Store Program", Font_7x10, true);
            break;
            
        case LOAD_PAGE  :
            // Завантаження програми
            display.SetCursor(5, 15);
            display.WriteString("Load Program", Font_7x10, true);
            break;
    }
    
    // Оновлюємо дисплей
    display.Update();
}

#endif