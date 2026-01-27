#include "menu.h"
#include "OLED_Greyscale_Daisy/fonts.h"
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

extern ParameterManager paramManager;

bool isBlink = false;
bool blinkStateChanged = false;
bool isStoreMode = false;
bool page_need_update = false;
uint8_t selModBlockIndex = 0;
uint8_t selSettingsBlockIndex = 0;
bool isModMatrixNeedUpdate = false;
bool isSettingsNeedUpdate = false;

char page_name[16] = "";
ActiveRow currentActiveRow = ROW_1; // Початково активний перший ряд

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

void DrawOneParamBlock(uint8_t blockIndex, ParamUnitName target_param, uint16_t textColor, uint16_t bgColor)
{

    Paint_NewImage(param_block_data[blockIndex].data, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0, bgColor);
    Paint_Clear(bgColor);
    // Paint_DrawCircle(PARAM_BLOCK_WIDTH / 2, PARAM_BLOCK_HEIGHT / 2, PARAM_BLOCK_WIDTH / 2, 0x04, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    ParamUnit param_unit = paramManager.GetUnit(target_param);

    float value = 0;
    char value_str[10];
    const char *label = "";
    const char *unit = "";

    if (currentPage == MAIN_PAGE)
    {
        value = paramManager.GetPhysical(currentPreset.mainSlots[blockIndex].target_param);
        label = paramManager.GetFullLabel(currentPreset.mainSlots[blockIndex].target_param);
    }
    else
    {
        if (paramSlots[blockIndex].target_param == P::NONE)
        {
            return;
        }
        value = paramManager.GetPhysical(paramSlots[blockIndex].target_param);
        label = paramManager.GetShortLabel(paramSlots[blockIndex].target_param);
    }

    if (param_unit == ParamUnit::PICTURE)
    {
        value = paramManager.GetPhysical(paramSlots[blockIndex].target_param);
        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, FONT_LIGHT_16, textColor, bgColor);
        DrawWaveformImage(value, true, (UBYTE)textColor);
    }
    else if (param_unit == ParamUnit::TEXT)
    {
        value = paramManager.GetValue(paramSlots[blockIndex].target_param);
        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, FONT_LIGHT_16, textColor, bgColor);
        DrawFilterModeText(value);
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
                if (value > 9.99f)
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
                if (value <= 100.0)
                {
                    if (value <=10.0)
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
                if (value > 9.99f)
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
        case ParamUnit::TEXT:
            unit = "";
            break;
        default:
            unit = "";
            break;
        }
        Paint_TextCentered(label, 0, PARAM_BLOCK_WIDTH, yBlockLabel, FONT_LIGHT_16, textColor, bgColor);
        Paint_TextCentered(value_str, 0, PARAM_BLOCK_WIDTH, yBlockValue, FONT_BOLD_18, textColor, bgColor);
        Paint_TextCentered(unit, 0, PARAM_BLOCK_WIDTH, yBlockUnit, FONT_LIGHT_12, textColor, bgColor);
    }

    if (currentPage == MAIN_PAGE)
    {
        if (currentPreset.mainSlots[blockIndex].isEditMode && isBlink)
        {
            Paint_DrawRectangle(1, 1, PARAM_BLOCK_WIDTH, PARAM_BLOCK_HEIGHT, 0x0f, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
            Paint_DrawRectangle(2, 2, PARAM_BLOCK_WIDTH - 1, PARAM_BLOCK_HEIGHT - 1, 0x08, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        }
        OLED_Transmit_DMA_Part(&param_block_data[blockIndex],
                               BLOCK_MAIN_X_START[blockIndex],
                               BLOCK_MAIN_Y_START[blockIndex],
                               BLOCK_MAIN_X_END[blockIndex],   
                               BLOCK_MAIN_Y_END[blockIndex]);
    }
    else
    {
        OLED_Transmit_DMA_Part(&param_block_data[blockIndex],
                               BLOCK_X_START[blockIndex],
                               BLOCK_Y_START[blockIndex],
                               BLOCK_X_END[blockIndex],
                               BLOCK_Y_END[blockIndex]);
    }
}

void DrawParamBlocks()
{

    for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        if (paramSlots[i].target_param == ParamUnitName::NONE)
        {
            continue;
        }

        bool isActiveRow = (i < 4 && currentActiveRow == ROW_1) || (i >= 4 && currentActiveRow == ROW_2);
        uint16_t textColor = isActiveRow ? 0xFF : 0x06; // Активні - білі, неактивні - темні
        uint16_t bgColor = BLACK;

        DrawOneParamBlock(i, paramSlots[i].target_param, textColor, bgColor);
    }
}

void DrawMainBlocks()
{

    for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
    {
        DrawOneParamBlock(i, currentPreset.mainSlots[i].target_param);
    }
}

void ToggleActiveRow()
{
    if (currentActiveRow == ROW_1)
    {
        if (paramSlots[4].target_param != P::NONE)
        {
            currentActiveRow = ROW_2;
        }
        else
        {
            return;
        }
    }
    else
    {
        currentActiveRow = ROW_1;
    }

    DrawParamPage(currentPage);
    DrawParamBlocks();
}

void UpdateEncoderSwitches()
{

    switch (currentPage)
    {
    case MAIN_PAGE:
    {
        static uint8_t currentEditSlot = UINT8_MAX;
        for (size_t i = 0; i < NUM_MAIN_SLOTS; i++)
        {
            if (!currentPreset.mainSlots[i].isEditMode)
            continue;
            if (currentEditSlot != i)
            {
                if (currentEditSlot < NUM_MAIN_SLOTS)
                {
                    currentPreset.mainSlots[currentEditSlot].isEditMode = false;
                    DrawOneParamBlock(currentEditSlot, currentPreset.mainSlots[currentEditSlot].target_param);
                }
                currentEditSlot = i;
                
            }
            // EditBlockParam(currentEditSlot);
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
        break;
    }
}

void EditBlockParam(uint8_t blockIndex)
{

    auto isDuplicate = [&](int val)
    {
        for (size_t i = 0; i < NUM_ENCODERS; i++)
        {
            if (i != blockIndex && (int)currentPreset.mainSlots[i].target_param == val)
                return true;
        }
        return false;
    };

    if (currentPreset.mainSlots[blockIndex].need_update)
    { // if encoder is turned

        int inc = encoderIncs[blockIndex];
        if (inc == 0)
        {
            currentPreset.mainSlots[blockIndex].need_update = false;
            return;
        }
        int dir = (inc > 0) ? 1 : -1;

        int value = (int)currentPreset.mainSlots[blockIndex].target_param;
        int temp_value = value;
        temp_value += dir;

        while (paramManager.GetUseInMain(static_cast<P>(temp_value)) == UseInMain::NONE
            || isDuplicate(temp_value))
        {
            temp_value += dir;
            if (temp_value >= (int)P::COUNT_PARAMS - 1 || temp_value <= (int)P::NONE)
            {
                temp_value = value;
                break;
            }
        }
        value = temp_value;

        if (value >= (int)P::COUNT_PARAMS - 1)
        {
            value = (int)P::COUNT_PARAMS - 1;
        }
        else if (value <= (int)P::NONE)
        {
            value = (int)P::NONE;
        }

        encoderIncs[blockIndex] = 0;
        currentPreset.mainSlots[blockIndex].target_param = (ParamUnitName)value;
        DrawOneParamBlock(blockIndex, currentPreset.mainSlots[blockIndex].target_param);
        currentPreset.mainSlots[blockIndex].need_update = false;
    }

    if (blinkStateChanged)
    {

        blinkStateChanged = false;
        DrawOneParamBlock(blockIndex, currentPreset.mainSlots[blockIndex].target_param);
    }
}

void UpdateMainSlots()
{
    for (size_t i = 0; i < 4; i++)
    {

        if (currentPreset.mainSlots[i].isEditMode)
        {
            EditBlockParam(i);
        }
        else if (currentPreset.mainSlots[i].need_update)
        {
            P paramName = currentPreset.mainSlots[i].target_param;

            paramManager.AdjustByIncrement(paramName, encoderIncs[i]); 
            DrawOneParamBlock(i, paramName, WHITE, BLACK);
            currentPreset.mainSlots[i].need_update = false;
            encoderIncs[i] = 0;
        }
    }
}

void UpdateParamSlots()
{
    for (size_t i = 0; i < 4; i++)
    {
        uint8_t paramIndex = GetActiveParamIndex(i);

        if (paramSlots[paramIndex].need_update)
        {
            P paramName = paramSlots[paramIndex].target_param;
            paramManager.AdjustByIncrement(paramName, encoderIncs[i]);

            bool isActiveRow = (paramIndex < 4 && currentActiveRow == ROW_1) ||
                               (paramIndex >= 4 && currentActiveRow == ROW_2);
            uint16_t textColor = isActiveRow ? WHITE : 0x02;

            DrawOneParamBlock(paramIndex, paramName, textColor, BLACK);
            paramSlots[paramIndex].need_update = false;
            encoderIncs[i] = 0;
        }
    }
}

void UpdateEncodersParams()
{
    if (isStoreMode)
    {
        EncoderChangeStore();
        return;
    }
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
    else if (currentPage == SETTINGS_PAGE)
    {
        EncoderChangeSettings();
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
        paramSlots[i].target_param = P::NONE;
    }
    switch (page)
    {
    case OSCILLATOR_1_PAGE:
        SetPageName("Oscillator 1");
        paramSlots[0].target_param = P::OSC_WAVEFORM_1;
        paramSlots[1].target_param = P::OSC_PITCH_1;
        paramSlots[2].target_param = P::OSC_DETUNE_1;
        paramSlots[3].target_param = P::OSC_AMP_1;
        paramSlots[4].target_param = P::OSC_PWM_1;
        break;
    case OSCILLATOR_2_PAGE:
        SetPageName("Oscillator 2");
        paramSlots[0].target_param = P::OSC_WAVEFORM_2;
        paramSlots[1].target_param = P::OSC_PITCH_2;
        paramSlots[2].target_param = P::OSC_DETUNE_2;
        paramSlots[3].target_param = P::OSC_AMP_2;
        paramSlots[4].target_param = P::OSC_PWM_2;
        break;
    case OSCILLATOR_3_PAGE:
        SetPageName("Oscillator 3");
        paramSlots[0].target_param = P::OSC_WAVEFORM_3;
        paramSlots[1].target_param = P::OSC_PITCH_3;
        paramSlots[2].target_param = P::OSC_DETUNE_3;
        paramSlots[3].target_param = P::OSC_AMP_3;
        paramSlots[4].target_param = P::OSC_PWM_3;
        break;
    case AMPLIFIER_PAGE:
        SetPageName("Amplifier");
        paramSlots[0].target_param = P::ADSR_ATTACK;
        paramSlots[1].target_param = P::ADSR_DECAY;
        paramSlots[2].target_param = P::ADSR_SUSTAIN;
        paramSlots[3].target_param = P::ADSR_RELEASE;
        paramSlots[4].target_param = P::MOD_ADSR_ATTACK;
        paramSlots[5].target_param = P::MOD_ADSR_DECAY;
        paramSlots[6].target_param = P::MOD_ADSR_SUSTAIN;
        paramSlots[7].target_param = P::MOD_ADSR_RELEASE;
        break;
    case FILTER_PAGE:
        SetPageName("Filter");
        paramSlots[0].target_param = P::FILTER_CUTOFF;
        paramSlots[1].target_param = P::FILTER_RESONANCE;
        paramSlots[2].target_param = P::FILTER_DRIVE;
        paramSlots[3].target_param = P::FILTER_MODE;
        break;
    case LFO_PAGE:
        SetPageName("LFO");
        paramSlots[0].target_param = P::MOD_LFO_WAVEFORM;
        paramSlots[1].target_param = P::MOD_LFO_FREQ;
        paramSlots[2].target_param = P::MOD_LFO_DEPTH;
        paramSlots[3].target_param = P::MOD_LFO_TRIGGER;
        break;
    case FX_PAGE:
        SetPageName("Effects");
        break;
    case OVERDRIVE_PAGE:
        SetPageName("Overdrive");
        paramSlots[0].target_param = P::EFFECT_OVERDRIVE_DRIVE;
        paramSlots[1].target_param = P::NONE;
        paramSlots[2].target_param = P::NONE;
        paramSlots[3].target_param = P::NONE;
        break;
    case CHORUS_PAGE:
        SetPageName("Chorus");  
        paramSlots[0].target_param = P::EFFECT_CHORUS_FREQ;
        paramSlots[1].target_param = P::EFFECT_CHORUS_DEPTH;
        paramSlots[2].target_param = P::EFFECT_CHORUS_FBK;
        paramSlots[3].target_param = P::EFFECT_CHORUS_DELAY;
        break;
    case COMPRESSOR_PAGE:
        SetPageName("Compressor");
        paramSlots[0].target_param = P::EFFECT_COMPRESSOR_ATTACK;
        paramSlots[1].target_param = P::EFFECT_COMPRESSOR_RELEASE;
        paramSlots[2].target_param = P::EFFECT_COMPRESSOR_THRESHOLD;
        paramSlots[3].target_param = P::EFFECT_COMPRESSOR_RATIO;
        // paramSlots[4].target_param = P::EFFECT_COMPRESSOR_MAKEUP;
        break;
    case FLANGER_PAGE:
        SetPageName("Flanger");
        paramSlots[0].target_param = P::EFFECT_FLANGER_LFO_FREQ;
        paramSlots[1].target_param = P::EFFECT_FLANGER_LFO_DEPTH;
        paramSlots[2].target_param = P::EFFECT_FLANGER_DELAY;
        paramSlots[3].target_param = P::EFFECT_FLANGER_FEEDBACK;
        break;
    case AUTOWAH_PAGE:
        SetPageName("Autowah");
        paramSlots[0].target_param = P::EFFECT_AUTOWAH_WAH; 
        paramSlots[1].target_param = P::EFFECT_AUTOWAH_LEVEL; 
        paramSlots[2].target_param = P::NONE;
        paramSlots[3].target_param = P::NONE;
        break;
    case REVERB_PAGE:
        SetPageName("Reverb");
        paramSlots[0].target_param = P::EFFECT_REVERB_FEEDBACK;
        paramSlots[1].target_param = P::EFFECT_REVERB_LPFREQ;
        paramSlots[2].target_param = P::NONE;
        paramSlots[3].target_param = P::NONE;
        break;
    case MOD_MATRIX_PAGE:
        SetPageName("Mod Matrix");
        break;
    case SETTINGS_PAGE:
        SetPageName("Settings");
        break;
    default:
        SetPageName(" - ");
        paramSlots[0].target_param = P::NONE;
        paramSlots[1].target_param = P::NONE;
        paramSlots[2].target_param = P::NONE;
        paramSlots[3].target_param = P::NONE;
        break;
    }
}

void EncoderChangeEffect()
{
    int dir_enc_value[2] = {encoderIncs[0], encoderIncs[3]};

    for (size_t i = 0; i < 2; i++)
    {
        if (currentPreset.effectSlots[i].need_update)
        {
            if (shift_pressed)
            {
                int newEffect = static_cast<int>(currentPreset.effectSlots[i].selectedEffect) + dir_enc_value[i];

                if ((i == 0 && currentPreset.effectSlots[1].selectedEffect == newEffect) || (i == 1 && currentPreset.effectSlots[0].selectedEffect == newEffect))
                {
                    newEffect += dir_enc_value[i];
                    if (newEffect >= EFFECT_COUNT - 1)
                    {
                        newEffect = EFFECT_COUNT - 1;
                    }
                    if ((i == 0 && currentPreset.effectSlots[1].selectedEffect == newEffect) || (i == 1 && currentPreset.effectSlots[0].selectedEffect == newEffect))
                    {
                        newEffect -= dir_enc_value[i];
                    }
                }
                if (newEffect < EFFECT_NONE)
                    newEffect = EFFECT_NONE;
                if (newEffect >= EFFECT_COUNT - 1)
                    newEffect = EFFECT_COUNT - 1;

                currentPreset.effectSlots[i].selectedEffect = static_cast<EffectName>(newEffect);
            }
            else
            {
                paramManager.AdjustByIncrement(EFFECT_SLOT_DRYWET[i], dir_enc_value[i]); 
            }
            DrawEffectBlock(i);
            currentPreset.effectSlots[i].need_update = false;
        }
    }
    encoderIncs[0] = encoderIncs[3] = 0;
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
        DrawModMatrixBlock(prevModBlockIndex);
        encoderIncs[0] = 0;
    }

    if (encoderIncs[1] != 0)
    {
        int dir = (encoderIncs[1] > 0) ? 1 : -1;
        int mod = (int)currentPreset.modMtx[selModBlockIndex].GetModSource();
        mod += dir;
        if (mod >= static_cast<int>(M::COUNT_MOD_SOURCES) - 1)
        {
            mod = static_cast<int>(M::COUNT_MOD_SOURCES) - 1;
        }
        if (mod < 0)
        {
            mod = 0;
        }
        currentPreset.modMtx[selModBlockIndex].SetModSource(static_cast<M>(mod));
        encoderIncs[1] = 0;
    }
    if (encoderIncs[2] != 0)
    {
        int dir = (encoderIncs[2] > 0) ? 1 : -1;
        float amount = currentPreset.modMtx[selModBlockIndex].GetModAmount();
        amount += dir * 0.01f;
        if (amount > 1.0f)
        {
            amount = 1.0f;
        }
        if (amount < -1.0f)
        {
            amount = -1.0f;
        }
        currentPreset.modMtx[selModBlockIndex].SetModAmount(amount);
        encoderIncs[2] = 0;
    }
    if (encoderIncs[3] != 0)
    {
        int dir = (encoderIncs[3] > 0) ? 1 : -1;
        int oldTarget = (int)currentPreset.modMtx[selModBlockIndex].GetModTarget();
        int target = oldTarget;
        target += dir;
        int shift = 0;

        while (paramManager.GetUseInMod(static_cast<P>(target)) != UseInMod::USED)
        {
            target += dir;
            shift += dir;
            
            if (target <= (int)P::NONE || target >= (int)P::COUNT_PARAMS - 1)
            {
                target = oldTarget;
            }
        }

        if (target >= (int)P::COUNT_PARAMS - 1)
        {
            target = (int)P::COUNT_PARAMS - 1;
        }
        else if (target <= (int)P::NONE)
        {
            target = (int)P::NONE;
        }
       
        paramManager.GetParam(static_cast<P>(oldTarget)).SetModifier(0.0f);
        currentPreset.modMtx[selModBlockIndex].SetModTarget((ParamUnitName)target);
        encoderIncs[3] = 0;
    }
    DrawModMatrixBlock(selModBlockIndex);
    isModMatrixNeedUpdate = false;
}

void EncoderChangeModMatrix()
{
    if (isModMatrixNeedUpdate)
    {
        EditModBlock();
    }
}

void EditSettingsBlock()
{
    if (encoderIncs[0] != 0)
    {
        int dir = (encoderIncs[0] > 0) ? 1 : -1;
        int prevSettingsBlockIndex = selSettingsBlockIndex;
        int value = prevSettingsBlockIndex;
        value += dir;
        if (value >= SETTINGS_BLOCKS_NUM)
        {
            value = SETTINGS_BLOCKS_NUM - 1;
        }
        if (value < 0)
        {
            value = 0;
        }
        selSettingsBlockIndex = value;
        DrawSettingsBlock(prevSettingsBlockIndex);
        encoderIncs[0] = 0;
    }

    if (encoderIncs[3] != 0)
    {
        int dir = (encoderIncs[3] > 0) ? 1 : -1;

        paramManager.AdjustByIncrement(SETTINGS_PARAMS[selSettingsBlockIndex], dir);
        encoderIncs[3] = 0;
    }
    DrawSettingsBlock(selSettingsBlockIndex);
    isSettingsNeedUpdate = false;
}

void EncoderChangeSettings()
{
    if (isSettingsNeedUpdate)
    {
        EditSettingsBlock();
    }
}

void InitSlots()
{
    currentPage = EMPTY;
    currentActiveRow = ROW_1;

    for (int i = 0; i < NUM_PARAM_BLOCKS; i++)
    {
        paramSlots[i].target_param = P::NONE;
        paramSlots[i].need_update = false;
    }
    for (int i = 0; i < NUM_MAIN_SLOTS; i++)
    {
        currentPreset.mainSlots[i].target_param = P::NONE;
        currentPreset.mainSlots[i].isEditMode = false;
        currentPreset.mainSlots[i].need_update = false;
    }

    for (int i = 0; i < NUM_FX_SLOTS; i++)
    {
        currentPreset.effectSlots[i].selectedEffect = EFFECT_NONE;
        currentPreset.effectSlots[i].label = "-";
        currentPreset.effectSlots[i].need_update = false;
        currentPreset.effectSlots[i].isActive = false;
    }

    for (size_t i = 0; i < MOD_MATRIX_BLOCKS_NUM; i++)
    {
        currentPreset.modMtx[i] = ModMatrix();
    }

    System::Delay(10);
}

void EncoderChangeStore()
{
    if (encoderIncs[4] != 0)
    {
        int dir = (encoderIncs[4] > 0) ? 1 : -1;
        int preset = currentPreset.number;
        preset += dir;
        if (preset < 0)
        {
            preset = 0;
        }
        if (preset >= PRESET_NUM)
        {
            preset = PRESET_NUM - 1;
        }
        currentPreset.number = preset;
        DrawStoreBlock();
        encoderIncs[4] = 0;
    }
}
