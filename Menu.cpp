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
using M = ModSource;

ParamSlot slots[NUM_PARAM_BLOCKS];
MenuSlot menu_slots[NUM_MAIN_SLOTS];
// ModMatrixSlot mod_matrix_slots[MOD_MATRIX_BLOCKS_NUM];
extern ParameterManager paramManager;
extern ModMatrix modMatrix[MOD_MATRIX_NUM];

const uint8_t yBlockLabel = 10;
const uint8_t yBlockValue = 30;
bool isBlink = false;
bool blinkStateChanged = false;
bool isStoreMode = false;
bool page_need_update = false;
uint8_t selModBlockIndex = 0;
bool isModMatrixNeedUpdate = false;

char page_name[16] = "";
ActiveRow currentActiveRow = ROW_1; // Початково активний перший ряд

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
    case 4: // OFF

        break;
    default:
        break;
    }
}

void InitOneParamBlock(uint8_t blockIndex, ParamUnitName target_param, uint16_t textColor, uint16_t bgColor)
{

    if (target_param == ParamUnitName::NONE)
    {
        return;
    }

    Paint_NewImage(param_block_data[blockIndex].data, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, bgColor);
    Paint_Clear(bgColor);

    ParamUnit param_unit = paramManager.GetParam(target_param).GetUnit();

    float value = 0;
    char value_str[10];
    const char *label = "";
    const char *unit = "";

    if (currentPage == MAIN_PAGE)
    {
        value = paramManager.GetPhysical(menu_slots[blockIndex].target_param);
        label = paramManager.GetLabel(menu_slots[blockIndex].target_param);
    }
    else
    {
        value = paramManager.GetPhysical(slots[blockIndex].target_param);
        label = paramManager.GetLabel(slots[blockIndex].target_param);
    }

    if (param_unit == ParamUnit::PICTURE)
    {
        value = paramManager.GetPhysical(slots[blockIndex].target_param);
        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, Font12, textColor, bgColor);
        DrawWaveformImage(value);
    }
    else
    {

        switch (param_unit)
        {
        case ParamUnit::HZ:
            if (value >= 1000)
            {
                value = value / 1000;
                unit = "kHz";
                if (value >= 10.0)
                {
                    sprintf(value_str, "%.1f", value);
                }
                else
                {
                    sprintf(value_str, "%.2f", value);
                }
            }
            else
            {
                unit = "Hz";
                if (value < 100.0)
                {
                    if (value < 10.0)
                    {
                        sprintf(value_str, "%.2f", value);
                    }
                    else
                    {
                        sprintf(value_str, "%.1f", value);
                    }
                }
                else
                {
                    sprintf(value_str, "%d", (int)value);
                }
            }
            break;
        case ParamUnit::SECONDS:
            if (value >= 1)
            {
                unit = "s";
                sprintf(value_str, "%.2f", value);
            }
            else
            {
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

    if (currentPage == MAIN_PAGE)
    {
        if (menu_slots[blockIndex].isEditMode && isBlink)
        {
            Paint_DrawRectangle(1, 2, 32, 50, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        }
        OLED_Part_Transmit_DMA(&param_block_data[blockIndex],
                               BLOCK_MAIN_X_START[blockIndex],
                               BLOCK_MAIN_Y_START[blockIndex],
                               BLOCK_MAIN_X_END[blockIndex],
                               BLOCK_MAIN_Y_END[blockIndex]);
    }
    else
    {
        OLED_Part_Transmit_DMA(&param_block_data[blockIndex],
                               BLOCK_X_START[blockIndex],
                               BLOCK_Y_START[blockIndex],
                               BLOCK_X_END[blockIndex],
                               BLOCK_Y_END[blockIndex]);
    }
}

void InitMainBlocks()
{

    for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
    {
        if (menu_slots[i].target_param == ParamUnitName::NONE)
        {
            continue;
        }
        InitOneParamBlock(i, menu_slots[i].target_param);
    }
}

void InitParamBlocks()
{

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        if (slots[i].target_param == ParamUnitName::NONE)
        {
            continue;
        }

        bool isActiveRow = (i < 4 && currentActiveRow == ROW_1) || (i >= 4 && currentActiveRow == ROW_2);
        uint16_t textColor = isActiveRow ? WHITE : 0x02; // Активні - білі, неактивні - темні
        uint16_t bgColor = BLACK;

        InitOneParamBlock(i, slots[i].target_param, textColor, bgColor);
    }
}

uint8_t GetActiveParamIndex(uint8_t encoderIndex)
{
    if (currentActiveRow == ROW_1)
    {
        return encoderIndex;
    }
    else
    {
        return encoderIndex + 4;
    }
}

void ToggleActiveRow()
{
    currentActiveRow = (currentActiveRow == ROW_1) ? ROW_2 : ROW_1;
    DrawParamPage(currentPage);
    InitParamBlocks();
}

void UpdateEncoderSwitches()
{

    switch (currentPage)
    {
    case MAIN_PAGE:
    {
        static uint8_t currentEditSlot = 0;
        bool init_block[NUM_MAIN_SLOTS] = {false, false, false, false};
        for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
        {

            if (menu_slots[i].isEditMode)
            {
                if (currentEditSlot == i)
                {
                    EditBlockParam(currentEditSlot);
                    continue;
                }
                else
                {

                    menu_slots[currentEditSlot].isEditMode = false;
                    InitOneParamBlock(currentEditSlot, menu_slots[currentEditSlot].target_param);
                    init_block[currentEditSlot] = true;
                    currentEditSlot = i;
                    menu_slots[currentEditSlot].isEditMode = true;
                    EditBlockParam(currentEditSlot);
                }
            }
            if (init_block[i])
            {
                InitOneParamBlock(i, menu_slots[i].target_param);
                init_block[i] = false;
            }
        }
        break;
    }
    case FX_PAGE:
    {
        if (encoderIncs[0] != 0 || encoderIncs[3] != 0)
        {
            EncoderChangeEffect();
            return;
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

void EditBlockParam(uint8_t blockIndex)
{

    auto isDuplicate = [&](int val)
    {
        for (size_t i = 0; i < NUM_ENCODERS; i++)
        {
            if (i != blockIndex && (int)menu_slots[i].target_param == val)
                return true;
        }
        return false;
    };

    if (menu_slots[blockIndex].need_update)
    { // if encoder is turned

        int inc = encoderIncs[blockIndex];
        if (inc == 0)
        {
            menu_slots[blockIndex].need_update = false;
            return;
        }
        int dir = (inc > 0) ? 1 : -1;

        int value = (int)menu_slots[blockIndex].target_param;
        value += dir;

        while (paramManager.GetUnit(static_cast<P>(value)) == ParamUnit::BOOL || isDuplicate(value))
        {
            value += dir;
        }
        if (value > (int)P::COUNT_PARAMS - 1)
        {
            value = (int)P::COUNT_PARAMS - 1;
        }
        else if (value <= (int)P::NONE + 1)
        {
            value = (int)P::NONE + 1;
        }

        encoderIncs[blockIndex] = 0;
        menu_slots[blockIndex].target_param = (ParamUnitName)value;
        InitOneParamBlock(blockIndex, menu_slots[blockIndex].target_param);
        menu_slots[blockIndex].need_update = false;
    }

    if (blinkStateChanged)
    {

        blinkStateChanged = false;
        InitOneParamBlock(blockIndex, menu_slots[blockIndex].target_param);
    }
}

void UpdateMainSlots()
{
    for (size_t i = 0; i < 4; i++)
    {

        if (menu_slots[i].isEditMode)
        {
            EditBlockParam(i);
        }
        else if (menu_slots[i].need_update)
        {
            P paramName = menu_slots[i].target_param;

            paramManager.GetParam(paramName).AdjustByIncrement(encoderIncs[i]);
            InitOneParamBlock(i, paramName, WHITE, BLACK);
            menu_slots[i].need_update = false;
            encoderIncs[i] = 0;
        }
    }
}

void UpdateParamSlots()
{
    for (size_t i = 0; i < 4; i++)
    {
        uint8_t paramIndex = GetActiveParamIndex(i);

        if (slots[paramIndex].need_update)
        {
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

void UpdateEncodersParams()
{
    if (currentPage == MAIN_PAGE)
    {
        UpdateMainSlots();
    }
    else if (currentPage == FX_PAGE)
    {
        EncoderChangeEffect();
    }
    else if (currentPage == MOD_MATRIX_PAGE)
    {
        EncoderChangeModMatrix();
    }
    else
    {
        UpdateParamSlots();
    }
}

void SetPageName(const char *name)
{
    strcpy(page_name, name);
}

void AssignParamsForPage(MenuPage page)
{

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        slots[i].target_param = P::NONE;
    }
    switch (page)
    {
    case OSCILLATOR_1_PAGE:
        SetPageName("Oscillator 1");
        slots[0].target_param = P::OSC_WAVEFORM_1;
        slots[1].target_param = P::OSC_PITCH_1;
        slots[2].target_param = P::OSC_DETUNE_1;
        slots[3].target_param = P::OSC_AMP_1;
        slots[4].target_param = P::OSC_PWM_1;
        break;
    case OSCILLATOR_2_PAGE:
        SetPageName("Oscillator 2");
        slots[0].target_param = P::OSC_WAVEFORM_2;
        slots[1].target_param = P::OSC_PITCH_2;
        slots[2].target_param = P::OSC_DETUNE_2;
        slots[3].target_param = P::OSC_AMP_2;
        slots[4].target_param = P::OSC_PWM_2;
        break;
    case OSCILLATOR_3_PAGE:
        SetPageName("Oscillator 3");
        slots[0].target_param = P::OSC_WAVEFORM_3;
        slots[1].target_param = P::OSC_PITCH_3;
        slots[2].target_param = P::OSC_DETUNE_3;
        slots[3].target_param = P::OSC_AMP_3;
        slots[4].target_param = P::OSC_PWM_3;
        break;
    case AMPLIFIER_PAGE:
        SetPageName("Amplifier");
        slots[0].target_param = P::ADSR_ATTACK;
        slots[1].target_param = P::ADSR_DECAY;
        slots[2].target_param = P::ADSR_SUSTAIN;
        slots[3].target_param = P::ADSR_RELEASE;
        slots[4].target_param = P::MOD_ADSR_ATTACK;
        slots[5].target_param = P::MOD_ADSR_DECAY;
        slots[6].target_param = P::MOD_ADSR_SUSTAIN;
        slots[7].target_param = P::MOD_ADSR_RELEASE;
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
        slots[0].target_param = P::MOD_LFO_WAVEFORM;
        slots[1].target_param = P::MOD_LFO_FREQ;
        slots[2].target_param = P::MOD_LFO_DEPTH;
        break;
    case FX_PAGE:
        SetPageName("Effects");
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
        slots[4].target_param = P::EFFECT_COMPRESSOR_MAKEUP;
        break;
    case REVERB_PAGE:
        SetPageName("Reverb");
        slots[0].target_param = P::EFFECT_REVERB_FEEDBACK;
        slots[1].target_param = P::EFFECT_REVERB_LPFREQ;
        break;
    case MOD_MATRIX_PAGE:
        SetPageName("Mod Matrix");
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

void EncoderChangeEffect()
{
    int dir_enc_value[2] = {encoderIncs[0], encoderIncs[3]};

    for (size_t i = 0; i < 2; i++)
    {
        if (effectSlot[i].need_update)
        {
            if (shift_pressed)
            {
                int newEffect = static_cast<int>(effectSlot[i].selectedEffect) + dir_enc_value[i];

                if ((i == 0 && effectSlot[1].selectedEffect == newEffect) || (i == 1 && effectSlot[0].selectedEffect == newEffect))
                {
                    newEffect += dir_enc_value[i];
                    if (newEffect >= EFFECT_COUNT - 1)
                    {
                        newEffect = EFFECT_COUNT - 1;
                    }
                    if ((i == 0 && effectSlot[1].selectedEffect == newEffect) || (i == 1 && effectSlot[0].selectedEffect == newEffect))
                    {
                        newEffect -= dir_enc_value[i];
                    }
                }
                if (newEffect < EFFECT_NONE)
                    newEffect = EFFECT_NONE;
                if (newEffect >= EFFECT_COUNT - 1)
                    newEffect = EFFECT_COUNT - 1;

                effectSlot[i].selectedEffect = static_cast<EffectName>(newEffect);
            }
            else
            {
                paramManager.GetParam(EFFECT_SLOT_DRYWET[i]).AdjustByIncrement(dir_enc_value[i]);
            }
            DrawEffectBlock(i);
            effectSlot[i].need_update = false;
        }
    }
    encoderIncs[0] = encoderIncs[3] = 0;
}

void InitModMatrixBlock(uint8_t blockIndex)
{
    Paint_NewImage(mod_matrix_block_data[blockIndex].data, MOD_MATRIX_BLOCK_WIDTH, MOD_MATRIX_BLOCK_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);

    Paint_TextCentered(modMatrix[blockIndex].GetModLabel(), 32, 64, 1, Font12, WHITE, BLACK);
    Paint_NumCentered(modMatrix[blockIndex].modAmount * 100, 64, 96, 1, 0, Font12, WHITE, BLACK);
    Paint_TextCentered(modMatrix[blockIndex].modTargetLabel, 96, 128, 1, Font12, WHITE, BLACK);
    if (blockIndex == selModBlockIndex)
    {
        uint8_t arrow_y = 7;
        Paint_DrawLine(4, arrow_y, 20, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(16, arrow_y - 3, 20, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        Paint_DrawLine(16, arrow_y + 3, 20, arrow_y, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        // Paint_DrawRectangle(0, 0, MOD_MATRIX_BLOCK_WIDTH, MOD_MATRIX_BLOCK_HEIGHT, 0x01, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    }

    OLED_Part_Transmit_DMA(&mod_matrix_block_data[blockIndex],
                           BLOCK_MOD_MATRIX_X_START,
                           BLOCK_MOD_MATRIX_Y_START[blockIndex],
                           BLOCK_MOD_MATRIX_X_END,
                           BLOCK_MOD_MATRIX_Y_END[blockIndex]);
}

void InitModMatrixBlocks()
{
    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        InitModMatrixBlock(i);
    }
}

void EditModBlock()
{
    if (encoderIncs[0] != 0)
    {
        int dir = (encoderIncs[0] > 0) ? 1 : -1;
        int prevModBlockIndex = selModBlockIndex;
        int value = prevModBlockIndex;
        value += dir;
        if (value >= MOD_MATRIX_BLOCKS_NUM)
        {
            value = MOD_MATRIX_BLOCKS_NUM - 1;
        }
        if (value < 0)
        {
            value = 0;
        }
        selModBlockIndex = value;
        InitModMatrixBlock(prevModBlockIndex);
        encoderIncs[0] = 0;
    }

    if (encoderIncs[1] != 0)
    {
        int dir = (encoderIncs[1] > 0) ? 1 : -1;
        int mod = (int)modMatrix[selModBlockIndex].modSourceType;
        mod += dir;
        if (mod >= static_cast<int>(M::COUNT_MOD_SOURCES) - 1)
        {
            mod = static_cast<int>(M::COUNT_MOD_SOURCES) - 1;
        }
        if (mod < 0)
        {
            mod = 0;
        }
        modMatrix[selModBlockIndex].modSourceType = static_cast<M>(mod);
        encoderIncs[1] = 0;
    }
    if (encoderIncs[2] != 0)
    {
        int dir = (encoderIncs[2] > 0) ? 1 : -1;
        float amount = modMatrix[selModBlockIndex].modAmount;
        amount += dir * 0.01f;
        if (amount > 1.0f)
        {
            amount = 1.0f;
        }
        if (amount < 0.0f)
        {
            amount = 0.0f;
        }
        modMatrix[selModBlockIndex].modAmount = amount;
        encoderIncs[2] = 0;
    }
    if (encoderIncs[3] != 0)
    {
        int dir = (encoderIncs[3] > 0) ? 1 : -1;
        int oldTarget = (int)modMatrix[selModBlockIndex].modTarget;
        int target = oldTarget;
        target += dir;
        while (paramManager.GetModulateableParam(static_cast<P>(target)) == ModulateableParam::NOT_MODULATABLE)
        {
            target += dir;
        }
        // ToDo: Переробити
        if (target >= (int)P::COUNT_PARAMS - 1)
        {
            if (paramManager.GetModulateableParam(static_cast<P>(target)) == ModulateableParam::NOT_MODULATABLE)
            {
                target -= 1;
            } else {
                target = (int)P::COUNT_PARAMS - 1;
            }
        }
        else if (target <= (int)P::NONE)
        {
            if (paramManager.GetModulateableParam(static_cast<P>(target)) == ModulateableParam::NOT_MODULATABLE)
            {
                target += 1;
            } else {
                target = (int)P::NONE;
            }
        }
        paramManager.GetParam(static_cast<P>(oldTarget)).SetModifier(0.0f);
        modMatrix[selModBlockIndex].modTarget = (ParamUnitName)target;
        modMatrix[selModBlockIndex].modTargetLabel = paramManager.GetLabel(modMatrix[selModBlockIndex].modTarget);
        encoderIncs[3] = 0;
    }
    InitModMatrixBlock(selModBlockIndex);
    isModMatrixNeedUpdate = false;
}

void EncoderChangeModMatrix()
{
    if (isModMatrixNeedUpdate)
    {
        EditModBlock();
    }
}

void InitSlots()
{
    currentPage = EMPTY;
    currentActiveRow = ROW_1;

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        slots[i].target_param = P::NONE;
        slots[i].need_update = false;
    }
    for (int i = 0; i < NUM_MAIN_SLOTS; i++)
    {
        menu_slots[i].isEditMode = false;
        menu_slots[i].need_update = false;
    }
    menu_slots[0].target_param = P::FILTER_CUTOFF;
    menu_slots[1].target_param = P::FILTER_RESONANCE;
    menu_slots[2].target_param = P::ADSR_ATTACK;
    menu_slots[3].target_param = P::ADSR_DECAY;

    effectSlot[0].selectedEffect = EFFECT_NONE;
    effectSlot[1].selectedEffect = EFFECT_REVERB;

    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        modMatrix[i].modTarget = P::NONE;
        modMatrix[i].modSource.value = 0.0f;
        modMatrix[i].modSource.label = "-";
        modMatrix[i].modAmount = 0.0f;
        modMatrix[i].modSourceType = M::NONE;
        modMatrix[i].modSource.label = "-";
        modMatrix[i].modTargetLabel = "-";
    }

    System::Delay(10);
}
