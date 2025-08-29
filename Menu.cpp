#include "menu.h"   
#include "display.h"

#include "parameters.h"
#include "effects.h"
#include "sx1509_expander.h"
#include "midi_handler.h"
#include "GUI_Paint.h"
#include "main.h"
#include "log_uart.h"

using P = ParamUnitName;

ParamSlot slots[NUM_PARAM_BLOCKS];
MenuSlot menu_slots[NUM_MAIN_SLOTS];
extern ParameterManager paramManager;

const uint8_t yBlockLabel = 10;
const uint8_t yBlockValue = 30;
bool isBlink = false;
bool blinkStateChanged = false;

char page_name[16] = "";
ActiveRow currentActiveRow = ROW_1;  // Початково активний перший ряд

void DrawWaveformImage(int waveform){
    
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
    case 4: // OFF
    
        break;
    default:
        break;
    }
}

void InitOneParamBlock(uint8_t blockIndex, ParamUnitName target_param, uint16_t textColor, uint16_t bgColor){

    if (target_param == ParamUnitName::NONE) {
        return;
    }

    Paint_NewImage(param_block_data[blockIndex].data, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, bgColor); 
    Paint_Clear(bgColor);

    ParamUnit param_unit = paramManager.GetParam(target_param).GetUnit();

    float value = 0; 
    char value_str[10];
    const char* label = "";
    const char* unit = "";

    if (currentPage == MAIN_PAGE) {
        value = paramManager.GetFloat(menu_slots[blockIndex].target_param);
        label = paramManager.GetLabel(menu_slots[blockIndex].target_param);
    } else {
        value = paramManager.GetFloat(slots[blockIndex].target_param);
        label = paramManager.GetLabel(slots[blockIndex].target_param);
    }

    if (param_unit == ParamUnit::PICTURE) {

        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, Font12, textColor, bgColor);
        DrawWaveformImage(value);

    } else {

        switch (param_unit) {
            case ParamUnit::HZ:
                if (value >= 1000) {
                    value = value / 1000;
                    unit = "kHz";
                    if (value >= 10.0) {
                        sprintf(value_str, "%.1f", value);
                    } else {
                        sprintf(value_str, "%.2f", value);
                    }
                } else {
                    unit = "Hz";
                    sprintf(value_str, "%d", (int)value);
                } 
                break;
            case ParamUnit::MS:
                
                if (value >= 1) {
                    unit = "s";
                    sprintf(value_str, "%.2f", value);

                } else {
                    value = value * 1000;
                    unit = "ms";
                    sprintf(value_str, "%d", (int)value);
                }
                break;
            case ParamUnit::SEMITONES:
                unit = "sem";
                sprintf(value_str, "%d", (int)value);
                break;
            case ParamUnit::CENTS:
                unit = "cen";
                sprintf(value_str, "%d", (int)value);
                break;
            case ParamUnit::PERCENT:
                unit = "%";
                sprintf(value_str, "%d", (int)value);
                break;
            case ParamUnit::UNITLESS:
                unit = "";
                break;
            default:
                unit = "";
                break;
        }        
        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, Font12, textColor, bgColor);
        Paint_TextCentered(value_str, 0, PARAM_BLOCK_WIDTH, yBlockValue - 4, Font12, textColor, bgColor);
        Paint_TextCentered(unit, 0, PARAM_BLOCK_WIDTH, yBlockValue + 10, Font8, textColor, bgColor);
    }

    if (currentPage == MAIN_PAGE) {
        if (menu_slots[blockIndex].isEditMode && isBlink) {
        Paint_DrawRectangle(1, 2, 32, 50, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        } 
        OLED_Part_Transmit_DMA(&param_block_data[blockIndex], 
            BLOCK_MAIN_X_START[blockIndex], 
            BLOCK_MAIN_Y_START[blockIndex], 
            BLOCK_MAIN_X_END[blockIndex], 
            BLOCK_MAIN_Y_END[blockIndex]);
    } else {
        OLED_Part_Transmit_DMA(&param_block_data[blockIndex], 
            BLOCK_X_START[blockIndex], 
            BLOCK_Y_START[blockIndex], 
            BLOCK_X_END[blockIndex], 
            BLOCK_Y_END[blockIndex]);
    }
}

void InitMainBlocks(){

    for (size_t i = 0; i < NUM_MAIN_SLOTS; i++) {
        if (menu_slots[i].target_param == ParamUnitName::NONE) {
            continue;
        }
        InitOneParamBlock(i, menu_slots[i].target_param);   
    }
}

void InitParamBlocks(){

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        if (slots[i].target_param == ParamUnitName::NONE) {
            continue;
        }
        
        bool isActiveRow = (i < 4 && currentActiveRow == ROW_1) || (i >= 4 && currentActiveRow == ROW_2);
        uint16_t textColor = isActiveRow ? WHITE : 0x02; // Активні - білі, неактивні - темні
        uint16_t bgColor = BLACK;
        
        InitOneParamBlock(i, slots[i].target_param, textColor, bgColor);   
    }
}

uint8_t GetActiveParamIndex(uint8_t encoderIndex) {
    if (currentActiveRow == ROW_1) {
        return encoderIndex;        
    } else {
        return encoderIndex + 4;    
    }
}

void ToggleActiveRow() {
    currentActiveRow = (currentActiveRow == ROW_1) ? ROW_2 : ROW_1;
    DrawParamPage(currentPage);
    InitParamBlocks();  
}

void UpdateEncoderSwitches() {

    switch (currentPage) {
        case MAIN_PAGE:
            for (size_t i = 0; i < NUM_MAIN_SLOTS; i++) {

                if (menu_slots[i].isEditMode) {
                    static uint8_t isCurrentEditSlot = 0;
                    if (isCurrentEditSlot != i) {
                        isCurrentEditSlot = i;

                        for (size_t j = 0; j < NUM_MAIN_SLOTS; j++) {
                            if (j != i) {
                                menu_slots[j].isEditMode = false;
                                InitOneParamBlock(j, menu_slots[j].target_param);
                        }
                    }
                }
                EditBlockParam(i);
            }
        }
        break;
        case FX_PAGE:
            if (encoderIncs[0] != 0 || encoderIncs[3] != 0) {
            EncoderChangeEffect();
            return;
        }
        break;
        default:
            break;  
    }
}

void EditBlockParam(uint8_t blockIndex) {
    
    if (menu_slots[blockIndex].need_update) { // if encoder is turned
        int value = (int)menu_slots[blockIndex].target_param;
            
        value += encoderIncs[blockIndex];
            
        if (value > 32) {
            value = 32;
        } else if (value < 0) {
            value = 0;
        }
        for (size_t i = 0; i < NUM_ENCODERS; i++) {
            if (i == blockIndex) continue;
            if (encoderIncs[blockIndex] == 1 && (ParamUnitName)value == menu_slots[i].target_param) {
                value++;
            }
            else if (encoderIncs[blockIndex] == -1 && (ParamUnitName)value == menu_slots[i].target_param) {
                value--;
            }
        }
        encoderIncs[blockIndex] = 0;
        
        menu_slots[blockIndex].target_param = (ParamUnitName)value;

        InitOneParamBlock(blockIndex, menu_slots[blockIndex].target_param);
        // reset need_update flag
        menu_slots[blockIndex].need_update = false;
    }
    UpdateBlinking(blockIndex);
}

void UpdateMainSlots() {
    for (size_t i = 0; i < 4; i++) {  
        
        if (menu_slots[i].isEditMode) {
            EditBlockParam(i);
            
        } else if (menu_slots[i].need_update) {  
            P paramName = menu_slots[i].target_param;
            
            paramManager.GetParam(paramName).AdjustByIncrement(encoderIncs[i]);
            InitOneParamBlock(i, paramName, WHITE, BLACK);
            menu_slots[i].need_update = false;
            encoderIncs[i] = 0;
        }
    }
}

void UpdateParamSlots() {
    for (size_t i = 0; i < 4; i++) {
        uint8_t paramIndex = GetActiveParamIndex(i);
        
        if (slots[paramIndex].need_update) {
            P paramName = slots[paramIndex].target_param;
            paramManager.GetParam(paramName).AdjustByIncrement(encoderIncs[i]);

            bool isActiveRow = (paramIndex < 4 && currentActiveRow == ROW_1) || 
                              (paramIndex >= 4 && currentActiveRow == ROW_2);
            uint16_t textColor = isActiveRow ? WHITE : 0x02;
            
            InitOneParamBlock(paramIndex, paramName, textColor, BLACK);
            slots[paramIndex].need_update = false;
            encoderIncs[i] = 0;
        }
    }
}

void UpdateEncodersParams() {
    if (currentPage == MAIN_PAGE) {
        UpdateMainSlots();
    } else {
        UpdateParamSlots();
    }
}

void SetPageName(const char* name) {
    strcpy(page_name, name);
}

void AssignParamsForPage(MenuPage page) {

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].target_param = P::NONE;
    }
    switch (page) {
        case OSCILLATOR_1_PAGE:
            SetPageName("Oscillator 1");
            slots[0].target_param = P::OSC_WAVEFORM_1;
            slots[1].target_param = P::OSC_PITCH_1;
            slots[2].target_param = P::OSC_DETUNE_1;
            slots[3].target_param = P::OSC_AMP_1;
            slots[4].target_param = P::OSC_PAN_1;
            slots[5].target_param = P::OSC_PWM_1;
            break;      
        case OSCILLATOR_2_PAGE:
            SetPageName("Oscillator 2");
            slots[0].target_param = P::OSC_WAVEFORM_2;
            slots[1].target_param = P::OSC_PITCH_2;
            slots[2].target_param = P::OSC_DETUNE_2;
            slots[3].target_param = P::OSC_AMP_2;
            slots[4].target_param = P::OSC_PAN_2;
            slots[5].target_param = P::OSC_PWM_2;
            break;      
        case OSCILLATOR_3_PAGE:
            SetPageName("Oscillator 3");
            slots[0].target_param = P::OSC_WAVEFORM_3;
            slots[1].target_param = P::OSC_PITCH_3;
            slots[2].target_param = P::OSC_DETUNE_3;
            slots[3].target_param = P::OSC_AMP_3;
            slots[4].target_param = P::OSC_PAN_3;
            slots[5].target_param = P::OSC_PWM_3;
            break;  
        case AMPLIFIER_PAGE:
            SetPageName("Amplifier");
            slots[0].target_param = P::ADSR_ATTACK;
            slots[1].target_param = P::ADSR_DECAY;
            slots[2].target_param = P::ADSR_SUSTAIN;
            slots[3].target_param = P::ADSR_RELEASE;
            break;  
        case FILTER_PAGE:
            SetPageName("Filter");
            slots[0].target_param = P::FILTER_CUTOFF;
            slots[1].target_param = P::FILTER_RESONANCE;
            slots[2].target_param = P::NONE;
            slots[3].target_param = P::NONE;
            break;  
        case LFO_PAGE:
            SetPageName("LFO");
            slots[0].target_param = P::LFO_WAVEFORM;
            slots[1].target_param = P::LFO_FREQ;
            slots[2].target_param = P::LFO_DEPTH;
          break;
        case FX_PAGE:
            
            break;
        case OVERDRIVE_PAGE:
            SetPageName("Overdrive");
            slots[0].target_param = P::EFFECT_OVERDRIVE_DRIVE;
            break;
        case CHORUS_PAGE: 
            SetPageName("Chorus");
            slots[0].target_param = P::EFFECT_CHORUS_FREQ;
            slots[1].target_param = P::EFFECT_CHORUS_DEPTH;
            slots[2].target_param = P::EFFECT_CHORUS_FBK;
            slots[3].target_param = P::EFFECT_CHORUS_DELAY;
            break;
        case COMPRESSOR_PAGE:
            SetPageName("Compressor");
            slots[0].target_param = P::EFFECT_COMPRESSOR_ATTACK;
            slots[1].target_param = P::EFFECT_COMPRESSOR_RELEASE;
            slots[2].target_param = P::EFFECT_COMPRESSOR_THRESHOLD;
            slots[3].target_param = P::EFFECT_COMPRESSOR_RATIO;
            break;
        case REVERB_PAGE:
            SetPageName("Reverb");
            slots[0].target_param = P::EFFECT_REVERB_DRYWET;
            slots[1].target_param = P::EFFECT_REVERB_FEEDBACK;
            slots[2].target_param = P::EFFECT_REVERB_LPFREQ;
            break;
        default: 
            SetPageName(" - ");
            slots[0].target_param = P::NONE;
            slots[1].target_param = P::NONE;
            slots[2].target_param = P::NONE;
            slots[3].target_param = P::NONE;
            break;
    }
}

void EncoderChangeEffect() {
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
    
void InitSlots() {
    currentPage = EMPTY;
    currentActiveRow = ROW_1;  // Скидаємо до першого ряду при ініціалізації
    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].target_param = P::NONE;
        slots[i].need_update = false;
    }
    for (int i = 0; i < NUM_MAIN_SLOTS; i++) {
        menu_slots[i].isEditMode = false;
        menu_slots[i].need_update = false;
    }
    menu_slots[0].target_param = P::FILTER_CUTOFF;
    menu_slots[1].target_param = P::FILTER_RESONANCE;
    menu_slots[2].target_param = P::ADSR_ATTACK;
    menu_slots[3].target_param = P::ADSR_DECAY;

    System::Delay(10);
}

void UpdateBlinking(uint8_t blockIndex) {
    if (!blinkStateChanged) return;  // Нічого не змінилось - виходимо
    
    blinkStateChanged = false;  // Скидаємо прапор
    InitOneParamBlock(blockIndex, menu_slots[blockIndex].target_param);
}

