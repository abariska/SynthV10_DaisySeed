#include "main.h"
#include "daisy.h"
#include "sx1509_expander.h"
#include "midi_handler.h"
#include "oscillator.h"
#include "display.h"
#include "log_uart.h"
#include "parameters.h"
#include "display.h"

using namespace daisy;

using P = ParamUnitName;
using M = ModSource;

DaisySeed hw;
TimerHandle timer_500ms;
TimerHandle timer_1ms;
CpuLoadMeter cpu_load;
ProcessType process_type;

extern Preset currentPreset;

int encoderIncs[5];
int test = 123;
float samplerate = 0;
bool shift_pressed = false;
float scope_data[128];
int scope_data_index = 0;
bool scope_data_ready = true;
bool scope_triggered = false;
float scope_prev_sample = 0.0f;
float scope_trigger_level = 0.0f;
int scope_trigger_delay = 0;
bool update_1ms = false;
bool update_500ms = false;
bool updateStoreLed = false;
uint8_t old_preset_number = 0;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size)
{
    cpu_load.OnBlockStart();
    static float scope_out = 0.0f;

    for (size_t i = 0; i < MOD_MATRIX_NUM; i++)
    {
        currentPreset.modMtx[i].RunMod();
    }

    for (size_t i = 0; i < size; i += 2)
    {
        float sig_after_fxL = 0.0f;
        float sig_after_fxR = 0.0f;
        float mix = 0.0f;
        float outL = 0.0f;
        float outR = 0.0f;

        ModSourcesProcess();
        VoiceProcess(mix);

        ProcessEffects(0, mix, mix, outL, outR);
        ProcessEffects(1, outL, outR, sig_after_fxL, sig_after_fxR);

        out[i] = sig_after_fxL * paramManager.GetValue(P::GLOBAL_MASTER_VOLUME);
        out[i + 1] = sig_after_fxR * paramManager.GetValue(P::GLOBAL_MASTER_VOLUME);

        scope_out = out[i] + out[i + 1];
    }
       if (!scope_triggered && 
        scope_prev_sample <= scope_trigger_level && 
        scope_out > scope_trigger_level)
    {
        scope_triggered = true;
        scope_data_index = 0;
        scope_trigger_delay = 0;
    }
    
    if (scope_triggered)
    {
        if (scope_trigger_delay > 2)
        {
            scope_data[scope_data_index] = scope_out;
            scope_data_index++;
            
            if (scope_data_index >= 128)
            {
                scope_data_ready = true;
                scope_triggered = false;
                scope_data_index = 0;
            }
        }
        scope_trigger_delay++;
    }
    
    scope_prev_sample = scope_out;
    
    cpu_load.OnBlockEnd();
}

int main(void)
{
    int blocksize = 16;

    hw.Configure();
    hw.Init(true);
    UartSerialInit();

    hw.SetAudioBlockSize(blocksize);
    samplerate = hw.AudioSampleRate();
    cpu_load.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

    OLED_Init();
    InitImages();
    DrawIntroPage();
    InitQSPI();

    InitSlots();
    InitSynthParams();
    SynthInit(samplerate, blocksize);
    MidiInit();

    InitSX1509Extenders();
    SetPage(MAIN_PAGE);

    hw.StartAudio(AudioCallback);

    Timer500ms();
    Timer1ms();
    System::Delay(10);
    process_type = PROCESS_CONTROLS;

    while (1)
    {
        UartMidiProcess();
        UsbMidiProcess();
        
        switch (process_type)
        {
            case PROCESS_CONTROLS:
                ProcessButtons();
                ProcessEncoders();
                UpdateEncodersParams();
                break;
            case UPDATE_PARAMS: 
                UpdateModSourcesParams();
                UpdateSynthParams();
                break;
            case PROCESS_DISPLAY:
                UpdatePage();
                DrawScope();
                break;
        }
        
        if (update_1ms)
        {
            UpdatePWMLeds();
            DrawVoicesBlock();

            update_1ms = false;
        }
        if (update_500ms)
        {
            CpuUsageDisplay();
            UpdateStoreLed();
            update_500ms = false;
        }
        process_type = (ProcessType)((process_type + 1) % 3);
    }
}

void ProcessButtons()
{
    bool any_button_change = sx1509_buttons.ReadAllPins();
    shift_pressed = sx1509_buttons.IsPressed(BUTTON_SHIFT);

    if (any_button_change)
    {
        if (isStoreMode)
        {
            if (shift_pressed)
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

        UpdateEncoderSwitches();

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
            { // Тільки 4 енкодери
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
            if (sx1509_buttons.isFallingEdge(BUTTON_BACK))
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
            if (sx1509_buttons.isFallingEdge(ENC_DIAL_SW))
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

void Callback500ms(void *data)
{
    isBlink = !isBlink;
    blinkStateChanged = true;
    update_500ms = true;
    // CpuUsageDisplay();
}

void Callback1ms(void *data)
{
    update_1ms = true;

}

void Timer1ms()
{
    TimerHandle::Config tim_1ms_cfg;

    tim_1ms_cfg.periph = TimerHandle::Config::Peripheral::TIM_3;
    tim_1ms_cfg.enable_irq = true;

    auto tim_target_freq = 10;
    auto tim_base_freq = System::GetPClk2Freq();
    tim_1ms_cfg.period = tim_base_freq / tim_target_freq;

    timer_1ms.Init(tim_1ms_cfg);
    timer_1ms.SetCallback(Callback1ms);
    timer_1ms.Start();

    System::Delay(10);
}

void Timer500ms()
{
    TimerHandle::Config tim_500ms_cfg;

    tim_500ms_cfg.periph = TimerHandle::Config::Peripheral::TIM_5;
    tim_500ms_cfg.enable_irq = true;

    auto tim_target_freq = 1;
    auto tim_base_freq = System::GetPClk2Freq();
    tim_500ms_cfg.period = tim_base_freq / tim_target_freq;

    timer_500ms.Init(tim_500ms_cfg);
    timer_500ms.SetCallback(Callback500ms);
    timer_500ms.Start();

    System::Delay(10);
}

void CpuUsageDisplay()
{

    // float cpu_avg_load = cpu_load.GetAvgCpuLoad() * 100;
    // UartPrintf("CPU load: ", cpu_avg_load);
    if (currentPage == MAIN_PAGE)
    {
        Paint_NewImage(cpu_load_block_data.data, 12, 12, 0, BLACK);
        Paint_Clear(BLACK);
        float cpu_avg_load = cpu_load.GetAvgCpuLoad() * 100;
        Paint_NumCentered(cpu_avg_load, 0, 12, 0, 0, Font8, WHITE, BLACK);
        OLED_Transmit_DMA_Part(&cpu_load_block_data, 116, 0, 128, 12);
        // UartPrint("CPU load: ", cpu_avg_load);
    }
    
    // else
    // {
    //     Paint_NewImage(cpu_load_block_data.data, 24, 24, 0, BLACK);
    //     Paint_Clear(BLACK);
    //     OLED_Part_Transmit_DMA(&cpu_load_block_data, 104, 0, 128, 24);
    // }
}

void DrawVoicesBlock()
{
    Paint_NewImage(voices_block_data.data, 20, 20, 0, BLACK);
    Paint_Clear(BLACK);
    
    int x1 = 1, x2 = 1;
    for (size_t i = 0; i < VOICE_NUM; i++)
    {
        x2 = x1 + 2;
        Paint_DrawLine(x1, 16, x2, 16, 0x01, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        if (voice[i].active)
        {
            Paint_DrawRectangle(x1, 2, x2, 12, 0x01, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        }
        x1 += 4;
    }
    OLED_Transmit_DMA_Part(&voices_block_data, 0, 0, 20, 16);
}
