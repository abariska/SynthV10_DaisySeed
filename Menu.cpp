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

const uint8_t yBlockLabel = 10;
const uint8_t yBlockValue = 30;

bool isParamEditMode[4] = {false, false, false, false};
char page_name[16] = "";

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
    OLED_Part_Transmit_DMA(&param_block_data[blockIndex], BLOCK_X_START[blockIndex], 
        BLOCK_TOP_LINE_Y, BLOCK_X_END[blockIndex], BLOCK_BOTTOM_LINE_Y);
}

void InitParamBlocks(){

    for (size_t i = 0; i < 4; i++) {
        if (slots[i].assignedParam == NONE) {
            continue;
        }
        InitOneParamBlock(i, *allParams[slots[i].assignedParam].target_param, allParams[slots[i].assignedParam].label);   
    }
}

void EditBlockParam(uint8_t blockIndex) {
    static uint32_t lastBlinkTime = 0;
    static bool blinkState = false;


    int value = (int)slots[blockIndex].assignedParam;
        
    value += encoderIncs[blockIndex];
        
    if (value > 32) {
        value = 32;
    } else if (value < 0) {
        value = 0;
    }
    for (size_t i = 0; i < ENCODER_NUM; i++) {
        if (i == blockIndex) continue;
        if (encoderIncs[blockIndex] == 1 && (ParamUnitName)value == slots[i].assignedParam) {
            value++;
        }
        else if (encoderIncs[blockIndex] == -1 && (ParamUnitName)value == slots[i].assignedParam) {
            value--;
        }
    }
    encoderIncs[blockIndex] = 0;
    
    int currentTime = System::GetNow();
    
    if (currentTime - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        lastBlinkTime = currentTime;
    }
    
    uint16_t textColor = blinkState ? BLACK : WHITE;
    uint16_t bgColor = blinkState ? 0x01 : BLACK;
    
    slots[blockIndex].assignedParam = (ParamUnitName)value;
    InitParam((ParamUnitName)value, blockIndex); 
    InitOneParamBlock(blockIndex, *allParams[slots[blockIndex].assignedParam].target_param, 
        allParams[slots[blockIndex].assignedParam].label, textColor, bgColor);
}

void UpdateEncoderSwitches() {

    switch (currentPage) {
        case MAIN_PAGE:
            for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {

                if (isParamEditMode[i]) {
                    static uint8_t isCurrentEditSlot = 0;

                    EditBlockParam(i);

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

void UpdateEncodersParams() {

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {

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
            encoderIncs[i] = 0;
        }
        slots[i].need_update = false;
    }
    
}

void SetPageName(const char* name) {
    strcpy(page_name, name);
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
        case OSCILLATOR_1_PAGE_2:
            SetPageName("Oscillator 1");
            InitParam(OSC_PAN_1, 0);
            break;
        case OSCILLATOR_2_PAGE:
            SetPageName("Oscillator 2");
            InitParam(OSC_WAVEFORM_2, 0);
            InitParam(OSC_PITCH_2, 1);
            InitParam(OSC_DETUNE_2, 2);
            InitParam(OSC_AMP_2, 3);
            break;      
        case OSCILLATOR_2_PAGE_2:
            SetPageName("Oscillator 2");
            InitParam(OSC_PAN_2, 0);
            break;
        case OSCILLATOR_3_PAGE:
            SetPageName("Oscillator 3");
            InitParam(OSC_WAVEFORM_3, 0);
            InitParam(OSC_PITCH_3, 1);
            InitParam(OSC_DETUNE_3, 2);
            InitParam(OSC_AMP_3, 3);
            break;  
        case OSCILLATOR_3_PAGE_2:
            SetPageName("Oscillator 3");
            InitParam(OSC_PAN_3, 0);
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
    { &params.osc[0].pan,      "Pan", 0, 1, 0.01, X100 },   //4 OSC_PAN_1
    { &params.osc[1].waveform, "Wav", 0, 3, 1, WAVEFORM },  //5 OSC_WAVEFORM_2
    { &params.osc[1].pitch,    "Sem", -36, 36, 1, REGULAR },   //6 OSC_PITCH_2
    { &params.osc[1].detune,   "Det", -0.5, 0.5, 0.01, X100 }, //7 OSC_DETUNE_2
    { &params.osc[1].amp,      "Amp", 0, 1, 0.01, X100 },   //8 OSC_AMP_2
    { &params.osc[1].pan,      "Pan", 0, 1, 0.01, X100 },   //9 OSC_PAN_2
    { &params.osc[2].waveform, "Wav", 0, 3, 1, WAVEFORM },  //8 OSC_WAVEFORM_3
    { &params.osc[2].pitch,    "Sem", -36, 36, 1, REGULAR },   //9 OSC_PITCH_3
    { &params.osc[2].detune,   "Det", -0.5, 0.5, 0.01, X100 }, //10 OSC_DETUNE_3
    { &params.osc[2].amp,      "Amp", 0, 1, 0.01, X100 },   //11 OSC_AMP_3
    { &params.osc[2].pan,      "Pan", 0, 1, 0.01, X100 },   //12 OSC_PAN_3
    { &params.adsr.attack,     "Atk", 0, 1, 0.01, X100 },   //12 ADSR_ATTACK
    { &params.adsr.decay,      "Dec", 0, 1, 0.01, X100 },   //13 ADSR_DECAY
    { &params.adsr.sustain,    "Sus", 0, 1, 0.01, X100 },   //14 ADSR_SUSTAIN
    { &params.adsr.release,    "Rel", 0, 10, 0.01, X100 },   //15 ADSR_RELEASE
    { &params.filter.cutoff,   "Cut", 50, 15000, 1, REGULAR }, //16 FILTER_CUTOFF
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
        allParams[param].target_param = nullptr;
        allParams[param].label = " - ";
        allParams[param].min = 0;
        allParams[param].max = 0;
        allParams[param].sensitivity = 0;
        allParams[param].valueType = REGULAR;
    }

}
    
void InitPageSlots() {
    currentPage = EMPTY;
    for (int i = 0; i < NUM_PARAM_BLOCKS; i++) {
        slots[i].assignedParam = NONE;
        slots[i].need_update = false;
    }
}