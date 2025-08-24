#include "menu.h"   
#include "display.h"

#include "parameters.h"
#include "effects.h"
#include "sx1509_expander.h"
#include "midi_handler.h"
#include "GUI_Paint.h"
#include "main.h"

MenuPage currentPage;
ParamUnitData allParams[ParamUnitName::NONE + 1];
ParamSlot slots[NUM_PARAM_BLOCKS];
MenuSlot menu_slots[NUM_MAIN_SLOTS];

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

void InitOneParamBlock(uint8_t blockIndex, float value, const char* label, uint16_t textColor, uint16_t bgColor){

    Paint_NewImage(param_block_data[blockIndex].data, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, bgColor); 
    Paint_Clear(bgColor);
    
    Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, Font12, textColor, bgColor);

    ParamUnitData* paramData = nullptr;
    if (currentPage == MAIN_PAGE) {
        paramData = &allParams[menu_slots[blockIndex].assignedParam];
    } else {
        paramData = &allParams[slots[blockIndex].assignedParam];
    }
    switch (paramData->valueType) {
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
        if (menu_slots[i].assignedParam == NONE) {
            continue;
        }
        InitOneParamBlock(i, *allParams[menu_slots[i].assignedParam].target_param, 
                         allParams[menu_slots[i].assignedParam].label);   
    }
}

void InitParamBlocks(){

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        if (slots[i].assignedParam == NONE) {
            continue;
        }
        
        bool isActiveRow = (i < 4 && currentActiveRow == ROW_1) || (i >= 4 && currentActiveRow == ROW_2);
        uint16_t textColor = isActiveRow ? WHITE : 0x02; // Активні - білі, неактивні - темні
        uint16_t bgColor = BLACK;
        
        InitOneParamBlock(i, *allParams[slots[i].assignedParam].target_param, 
                            allParams[slots[i].assignedParam].label, textColor, bgColor);   
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
                                InitOneParamBlock(j, *allParams[menu_slots[j].assignedParam].target_param, 
                                    allParams[menu_slots[j].assignedParam].label, WHITE, BLACK);
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
        int value = (int)menu_slots[blockIndex].assignedParam;
            
        value += encoderIncs[blockIndex];
            
        if (value > 32) {
            value = 32;
        } else if (value < 0) {
            value = 0;
        }
        for (size_t i = 0; i < NUM_ENCODERS; i++) {
            if (i == blockIndex) continue;
            if (encoderIncs[blockIndex] == 1 && (ParamUnitName)value == menu_slots[i].assignedParam) {
                value++;
            }
            else if (encoderIncs[blockIndex] == -1 && (ParamUnitName)value == menu_slots[i].assignedParam) {
                value--;
            }
        }
        encoderIncs[blockIndex] = 0;
        
        menu_slots[blockIndex].assignedParam = (ParamUnitName)value;

        AssignParam(menu_slots[blockIndex].assignedParam, blockIndex); 
        InitOneParamBlock(blockIndex, *allParams[menu_slots[blockIndex].assignedParam].target_param, 
            allParams[menu_slots[blockIndex].assignedParam].label, WHITE, BLACK);
        // reset need_update flag
        menu_slots[blockIndex].need_update = false;
    }
    UpdateBlinking(blockIndex);
}



void UpdateParamValue(uint8_t encoderIndex, ParamUnitName paramName, float* target_param) {

    if (target_param == nullptr) return;

    float value = *target_param;
    value += encoderIncs[encoderIndex] * allParams[paramName].sensitivity;

    if (value > allParams[paramName].max) {
        value = allParams[paramName].max;
    } else if (value < allParams[paramName].min) {
        value = allParams[paramName].min;
    }
    *target_param = value;
    encoderIncs[encoderIndex] = 0;
}

void UpdateMainSlots() {
    for (size_t i = 0; i < 4; i++) {  
        
        if (menu_slots[i].isEditMode) {
            EditBlockParam(i);
            
        } else if (menu_slots[i].need_update) {  
            ParamUnitName paramName = menu_slots[i].assignedParam;
            float* target_param = allParams[paramName].target_param;
            
            UpdateParamValue(i, paramName, target_param);
            InitOneParamBlock(i, *target_param, allParams[paramName].label, WHITE, BLACK);
            menu_slots[i].need_update = false;
        }
    }
}

void UpdateParamSlots() {
    for (size_t i = 0; i < 4; i++) {
        uint8_t paramIndex = GetActiveParamIndex(i);
        
        if (slots[paramIndex].need_update) {
            ParamUnitName paramName = slots[paramIndex].assignedParam;
            float* target_param = allParams[paramName].target_param;
            
            UpdateParamValue(i, paramName, target_param);
            
            bool isActiveRow = (paramIndex < 4 && currentActiveRow == ROW_1) || 
                              (paramIndex >= 4 && currentActiveRow == ROW_2);
            uint16_t textColor = isActiveRow ? WHITE : 0x02;
            
            InitOneParamBlock(paramIndex, *target_param, allParams[paramName].label, textColor, BLACK);
            slots[paramIndex].need_update = false;
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

void AssignMainParams(){
    
    SetPageName("");
    for (int i = 0; i < NUM_MAIN_SLOTS; i++) {
        AssignParam((ParamUnitName)menu_slots[i].assignedParam, i);
    }
}

void AssignParamsForPage(MenuPage page) {

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].assignedParam = NONE;
    }
    switch (page) {
        case OSCILLATOR_1_PAGE:
            SetPageName("Oscillator 1");
            AssignParam(OSC_WAVEFORM_1, 0);
            AssignParam(OSC_PITCH_1, 1);
            AssignParam(OSC_DETUNE_1, 2);
            AssignParam(OSC_AMP_1, 3);
            AssignParam(OSC_PAN_1, 4);
            break;      
        case OSCILLATOR_2_PAGE:
            SetPageName("Oscillator 2");
            AssignParam(OSC_WAVEFORM_2, 0);
            AssignParam(OSC_PITCH_2, 1);
            AssignParam(OSC_DETUNE_2, 2);
            AssignParam(OSC_AMP_2, 3);
            AssignParam(OSC_PAN_2, 4);
            break;      
        case OSCILLATOR_3_PAGE:
            SetPageName("Oscillator 3");
            AssignParam(OSC_WAVEFORM_3, 0);
            AssignParam(OSC_PITCH_3, 1);
            AssignParam(OSC_DETUNE_3, 2);
            AssignParam(OSC_AMP_3, 3);
            AssignParam(OSC_PAN_3, 4);
            break;  
        case AMPLIFIER_PAGE:
            SetPageName("Amplifier");
            AssignParam(ADSR_ATTACK, 0);
            AssignParam(ADSR_DECAY, 1);
            AssignParam(ADSR_SUSTAIN, 2);
            AssignParam(ADSR_RELEASE, 3);
            break;  
        case FILTER_PAGE:
            SetPageName("Filter");
            AssignParam(FILTER_CUTOFF, 0);
            AssignParam(FILTER_RESONANCE, 1);
            AssignParam(NONE, 2);
            AssignParam(NONE, 3);
            break;  
        case LFO_PAGE:
            SetPageName("LFO");
            AssignParam(LFO_WAVEFORM, 0);
            AssignParam(LFO_FREQ, 1);
            AssignParam(LFO_DEPTH, 2);
          break;
        case FX_PAGE:
            
            break;
        case OVERDRIVE_PAGE:
            SetPageName("Overdrive");
            AssignParam(EFFECT_OVERDRIVE_DRIVE, 0);
            break;
        case CHORUS_PAGE: 
            SetPageName("Chorus");
            AssignParam(EFFECT_CHORUS_FREQ, 0);
            AssignParam(EFFECT_CHORUS_DEPTH, 1);
            AssignParam(EFFECT_CHORUS_FBK, 2);
            AssignParam(EFFECT_CHORUS_DELAY, 3);
            break;
        case COMPRESSOR_PAGE:
            SetPageName("Compressor");
            AssignParam(EFFECT_COMPRESSOR_ATTACK, 0);
            AssignParam(EFFECT_COMPRESSOR_RELEASE, 1);
            AssignParam(EFFECT_COMPRESSOR_THRESHOLD, 2);
            AssignParam(EFFECT_COMPRESSOR_RATIO, 3);
            break;
        case REVERB_PAGE:
            SetPageName("Reverb");
            AssignParam(EFFECT_REVERB_FBK, 0);
            AssignParam(EFFECT_REVERB_LPFREQ, 1);
            AssignParam(EFFECT_REVERB_DRYWET, 2);
            break;
        default: 
            SetPageName(" - ");
            AssignParam(NONE, 0);
            AssignParam(NONE, 1);
            AssignParam(NONE, 2);
            AssignParam(NONE, 3);
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

// Array of parameter initialization data
const ParamUnitData DSY_SDRAM_DATA paramInitTable[] = {
    { &params.osc[0].waveform, "Wav", 0, 3, 1, WAVEFORM },   // 0 OSC_WAVEFORM_1
    { &params.osc[0].pitch,    "Sem", -36, 36, 1, REGULAR },   // 1 OSC_PITCH_1
    { &params.osc[0].detune,   "Det", -0.5, 0.5, 0.01, X100 }, //2 OSC_DETUNE_1
    { &params.osc[0].amp,      "Amp", 0, 1, 0.01, X100 },   //3 OSC_AMP_1
    { &params.osc[0].pan,      "Pan", -1.0, 1.0, 0.01, X100 },   //4 OSC_PAN_1
    { &params.osc[1].waveform, "Wav", 0, 3, 1, WAVEFORM },  //4 OSC_WAVEFORM_2
    { &params.osc[1].pitch,    "Sem", -36, 36, 1, REGULAR },   //5 OSC_PITCH_2
    { &params.osc[1].detune,   "Det", -0.5, 0.5, 0.01, X100 }, //6 OSC_DETUNE_2
    { &params.osc[1].amp,      "Amp", 0, 1, 0.01, X100 },   //7 OSC_AMP_2
    { &params.osc[1].pan,      "Pan", -1.0, 1.0, 0.01, X100 },   //8 OSC_PAN_2
    { &params.osc[2].waveform, "Wav", 0, 3, 1, WAVEFORM },  //9 OSC_WAVEFORM_3
    { &params.osc[2].pitch,    "Sem", -36, 36, 1, REGULAR },   //9 OSC_PITCH_3
    { &params.osc[2].detune,   "Det", -0.5, 0.5, 0.01, X100 }, //10 OSC_DETUNE_3
    { &params.osc[2].amp,      "Amp", 0, 1, 0.01, X100 },   //11 OSC_AMP_3
    { &params.osc[2].pan,      "Pan", -1.0, 1.0, 0.01, X100 },   //12 OSC_PAN_3
    { &params.adsr.attack,     "Atk", 0, 1, 0.01, X100 },   //12 ADSR_ATTACK
    { &params.adsr.decay,      "Dec", 0, 1, 0.01, X100 },   //13 ADSR_DECAY
    { &params.adsr.sustain,    "Sus", 0, 1, 0.01, X100 },   //14 ADSR_SUSTAIN
    { &params.adsr.release,    "Rel", 0, 10, 0.01, X100 },   //15 ADSR_RELEASE
    { &params.filter.cutoff,   "Cut", 50, 15000, 10, REGULAR }, //16 FILTER_CUTOFF
    { &params.filter.resonance,"Res", 0, 1, 0.01, X100 },   //17 FILTER_RESONANCE
    { &params.lfo.waveform,    "Wav", 0, 3, 1, WAVEFORM },  //18 LFO_WAVEFORM
    { &params.lfo.freq,        "Freq", 0, 1, 0.01, X100 },//19 LFO_FREQ
    { &params.lfo.depth,       "Dpth", 0, 1, 0.01, X100 },  //20 LFO_DEPTH
    { &params.overdriveParams.drive, "Drv", 0, 1, 0.01, X100 }, //21 EFFECT_OVERDRIVE_DRIVE
    { &params.chorusParams.freq,     "Freq", 0, 1, 0.01, REGULAR }, //22 EFFECT_CHORUS_FREQ
    { &params.chorusParams.depth,    "Dpth", 0, 1, 0.01, X100 },   //23 EFFECT_CHORUS_DEPTH
    { &params.chorusParams.feedback, "Fbk", 0, 1, 0.01, X100 },   //24 EFFECT_CHORUS_FBK
    { &params.chorusParams.delay,    "Dly", 0, 1, 0.01, X100 },   //25 EFFECT_CHORUS_PAN (тимчасово використовуємо delay)
    { &params.compressorParams.attack,    "Atk", 0, 1, 0.01, X100 }, //26 EFFECT_COMPRESSOR_ATTACK
    { &params.compressorParams.release,   "Rel", 0, 1, 0.01, X100 }, //27 EFFECT_COMPRESSOR_RELEASE
    { &params.compressorParams.threshold, "Thr", 0, 1, 0.01, X100 }, //28 EFFECT_COMPRESSOR_THRESHOLD
    { &params.compressorParams.ratio,     "Ratio",0, 1, 0.01, X100 }, //29 EFFECT_COMPRESSOR_RATIO
    { &params.reverbParams.dryWet,   "Dry", 0, 1, 0.01, X100 },   //30 EFFECT_REVERB_DRYWET
    { &params.reverbParams.feedback, "Fbk", 0, 1, 0.01, X100 },   //31 EFFECT_REVERB_FBK
    { &params.reverbParams.lpFreq,   "LPF", 0, 1, 0.01, X100 },    //32 EFFECT_REVERB_LPFREQ
};

void AssignParam(ParamUnitName param, uint8_t slotIndex) {

    if (currentPage == MAIN_PAGE) {
        menu_slots[slotIndex].assignedParam = param;
    } else {
        slots[slotIndex].assignedParam = param;
    }
    if (param < NONE && param < sizeof(paramInitTable)/sizeof(paramInitTable[0])) {
        allParams[param].target_param = paramInitTable[param].target_param;
        allParams[param].label = paramInitTable[param].label;
        allParams[param].min = paramInitTable[param].min;
        allParams[param].max = paramInitTable[param].max;
        allParams[param].sensitivity = paramInitTable[param].sensitivity;
        allParams[param].valueType = paramInitTable[param].valueType;
    } else {
        allParams[param].target_param = nullptr;
        allParams[param].label = " - ";
        allParams[param].min = 0;
        allParams[param].max = 0;
        allParams[param].sensitivity = 0;
        allParams[param].valueType = REGULAR;
    }
}
    
void InitSlots() {
    currentPage = EMPTY;
    currentActiveRow = ROW_1;  // Скидаємо до першого ряду при ініціалізації
    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].assignedParam = NONE;
        slots[i].need_update = false;
    }
    for (int i = 0; i < NUM_MAIN_SLOTS; i++) {
        menu_slots[i].isEditMode = false;
        menu_slots[i].need_update = false;
    }
    menu_slots[0].assignedParam = FILTER_CUTOFF;
    menu_slots[1].assignedParam = FILTER_RESONANCE;
    menu_slots[2].assignedParam = ADSR_ATTACK;
    menu_slots[3].assignedParam = ADSR_DECAY;

    System::Delay(10);
}

void UpdateBlinking(uint8_t blockIndex) {
    if (!blinkStateChanged) return;  // Нічого не змінилось - виходимо
    
    blinkStateChanged = false;  // Скидаємо прапор
    InitOneParamBlock(blockIndex, *allParams[menu_slots[blockIndex].assignedParam].target_param, 
        allParams[menu_slots[blockIndex].assignedParam].label, WHITE, BLACK);
}
