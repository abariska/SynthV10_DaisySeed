#include "controls.h"
#include "menu.h"
#include "globals.h"

int encoderIncs[5];
bool shift_pressed = false;

uint8_t old_preset_number = 0;

void ProcessButtons()
{
    bool any_button_change = sx1509_buttons.ReadAllPins();
    shift_pressed = sx1509_buttons.IsPressed(BUTTON_SHIFT);

    if (any_button_change)
    {
        if (isStoreMode)
        {
            if (sx1509_buttons.isFallingEdge(BUTTON_EXIT))
            {
                isStoreMode = false;
                currentPreset.number = old_preset_number;
                page_need_update = true;
                UpdatePage();
            } 
            if (sx1509_buttons.isFallingEdge(BUTTON_STORE))
            {
                SavePreset(currentPreset.number, currentPreset);
                page_need_update = true;
                isStoreMode = false;
                old_preset_number = currentPreset.number;
                UpdatePage();
            }
            return;
        }



        if (currentPage == MenuPage::FX_PAGE)
        {
            if (shift_pressed)
            {
                if (sx1509_buttons.isFallingEdge(ENC_1_SW))
                {
                    currentPreset.effectSlots[0].isActive = !currentPreset.effectSlots[0].isActive;
                    DrawEffectBlock(0);
                }
                if (sx1509_buttons.isFallingEdge(ENC_4_SW))
                {
                    currentPreset.effectSlots[1].isActive = !currentPreset.effectSlots[1].isActive;
                    DrawEffectBlock(1);
                }
            }
            else
            {
                if (sx1509_buttons.isFallingEdge(ENC_1_SW))
                {
                    SelectEffectPage(0);
                }
                if (sx1509_buttons.isFallingEdge(ENC_4_SW))
                {
                    SelectEffectPage(1);
                }
            }
        }
        else if (currentPage == MenuPage::MAIN_PAGE)
        {
            for (size_t i = 0; i < 4; i++)
            { 
                if (sx1509_buttons.isFallingEdge(ENC_1_SW + i))
                {
                    currentPreset.mainSlots[i].isEditMode = !currentPreset.mainSlots[i].isEditMode;
                    if (!currentPreset.mainSlots[i].isEditMode)
                    {
                        DrawOneParamBlock(i, currentPreset.mainSlots[i].target_param, WHITE, BLACK);
                    }
                } else if (shift_pressed && currentPreset.mainSlots[i].isEditMode)
                {
                    currentPreset.mainSlots[i].isEditMode = false;
                    DrawOneParamBlock(i, currentPreset.mainSlots[i].target_param, WHITE, BLACK);
                }
            }
        }

        if (shift_pressed)
        {
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_1))
            {
                bool osc_active_1 = paramManager.GetValue(P::OSC_ACTIVE_1);
                paramManager.SetBool(P::OSC_ACTIVE_1, !osc_active_1);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_2))
            {
                bool osc_active_2 = paramManager.GetValue(P::OSC_ACTIVE_2);
                paramManager.SetBool(P::OSC_ACTIVE_2, !osc_active_2);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_3))
            {
                bool osc_active_3 = paramManager.GetValue(P::OSC_ACTIVE_3);
                paramManager.SetBool(P::OSC_ACTIVE_3, !osc_active_3);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_STORE))
            {
                page_need_update = true;
                ResetPreset(currentPreset.number);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_MTX))
            {
                SetPage(MenuPage::SETTINGS_PAGE);
            }
        }
        else
        {
            if (sx1509_buttons.isFallingEdge(BUTTON_EXIT))
            {
                SetPage(MenuPage::MAIN_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_1))
            {
                if (currentPage == MenuPage::OSCILLATOR_1_PAGE)
                {
                    ToggleActiveRow(); // Перемикання між рядами на тій же сторінці
                }
                else
                {
                    SetPage(MenuPage::OSCILLATOR_1_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_2))
            {
                if (currentPage == MenuPage::OSCILLATOR_2_PAGE)
                {
                    ToggleActiveRow();
                }
                else
                {
                    SetPage(MenuPage::OSCILLATOR_2_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_3))
            {
                if (currentPage == MenuPage::OSCILLATOR_3_PAGE)
                {
                    ToggleActiveRow();
                }
                else
                {
                    SetPage(MenuPage::OSCILLATOR_3_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_FLT))
            {
                if (currentPage == MenuPage::FILTER_PAGE)
                {
                    ToggleActiveRow();
                }
                else
                {
                    SetPage(MenuPage::FILTER_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_AMP))
            {
                if (currentPage == MenuPage::AMPLIFIER_PAGE)
                {
                    ToggleActiveRow();
                }
                else
                {
                    SetPage(MenuPage::AMPLIFIER_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_FX))
            {
                SetPage(MenuPage::FX_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_LFO))
            {
                if (currentPage == MenuPage::LFO_PAGE)
                {
                    ToggleActiveRow();
                }
                else
                {
                    SetPage(MenuPage::LFO_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_MTX))
            {
                SetPage(MenuPage::MOD_MATRIX_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_SETTINGS))
            {
                SetPage(MenuPage::SETTINGS_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_STORE))
            {
                isStoreMode = true;
                old_preset_number = currentPreset.number;
                DrawStoreBlock();                    
            }
        }
    }
}
void ProcessEncoders()
{

    bool any_pin_change = sx1509_encoders.ReadAllPins();

    if (any_pin_change)
    {
        encoderIncs[0] = EncoderInc(ENC_1_A, ENC_1_B);
        encoderIncs[1] = EncoderInc(ENC_2_A, ENC_2_B);
        encoderIncs[2] = EncoderInc(ENC_3_A, ENC_3_B);
        encoderIncs[3] = EncoderInc(ENC_4_A, ENC_4_B);
        encoderIncs[4] = EncoderInc(ENC_DIAL_A, ENC_DIAL_B);
    }

    if (!isStoreMode)
    {
        if (encoderIncs[4] != 0)
        {
            uint8_t newPresetNum = currentPreset.number + encoderIncs[4];
            if (newPresetNum < 0 || newPresetNum > PRESET_NUM - 1)
            {
                return;
            }
            else
            {
                ApplyPreset(newPresetNum);
            }
            encoderIncs[4] = 0;
            }
        switch (currentPage)
        {
        case MAIN_PAGE:
            for (size_t i = 0; i < NUM_ENCODERS; i++)
            { // Only 4 encoders
                if (encoderIncs[i] != 0)
                {
                    currentPreset.mainSlots[i].need_update = true;
                }
            }
            break;
        case FX_PAGE:
            if (encoderIncs[0] != 0)
            {
                currentPreset.effectSlots[0].need_update = true;
            }
            if (encoderIncs[3] != 0)
            {
                currentPreset.effectSlots[1].need_update = true;
            }
            break;
        case MOD_MATRIX_PAGE:
        
            if (encoderIncs[0] != 0 || encoderIncs[1] != 0 || encoderIncs[2] != 0 || encoderIncs[3] != 0)
            {
                isModMatrixNeedUpdate = true;
            }
            break;
        case SETTINGS_PAGE:
            if (encoderIncs[0] != 0 || encoderIncs[1] != 0 || encoderIncs[2] != 0 || encoderIncs[3] != 0)
            {
                isSettingsNeedUpdate = true;
            }
            break;
        default:
            for (size_t i = 0; i < 4; i++)
            { // Only 4 encoders
                if (encoderIncs[i] != 0)
                {
                    uint8_t paramIndex = GetActiveParamIndex(i); // Get index of active parameter
                    paramSlots[paramIndex].need_update = true;
                }
            }
            break;
        }
    }
}