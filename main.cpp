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

DaisySeed hw;
TimerHandle tim_display;
CpuLoadMeter cpu_load;

extern Preset currentPreset;

int encoderIncs[5];
int test = 123;
float samplerate = 0;
bool update_for_preset_needed = false;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size)
{
    cpu_load.OnBlockStart();

    midiUart.Listen();
    midiUsb.Listen();

    while (midiUsb.HasEvents())
    {
        auto msg = midiUsb.PopEvent();
        HandleMidiMessage(msg);
    }

    while (midiUart.HasEvents())
    {
        auto msg = midiUart.PopEvent();
        HandleMidiMessage(msg);
    }

    for (size_t i = 0; i < size; i += 2)
    {
        float sig_after_fxL = 0.0f;
        float sig_after_fxR = 0.0f;
        float mix = 0.0f;
        float outL = 0.0f;
        float outR = 0.0f;

        VoiceProcess(mix);

        ProcessEffects(effectSlot[0], mix, mix, outL, outR);
        ProcessEffects(effectSlot[1], outL, outR, sig_after_fxL, sig_after_fxR);

        out[i] = sig_after_fxL * 0.5f;
        out[i + 1] = sig_after_fxR * 0.5f;
    }
    cpu_load.OnBlockEnd();
}

int main(void)
{
    int blocksize = 4;

    hw.Configure();
    hw.Init();
    UartSerialInit();

    hw.SetAudioBlockSize(blocksize);
    samplerate = hw.AudioSampleRate();
    cpu_load.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

    OLED_1in5_Init();
    InitImages();
    DrawIntroPage();
    System::Delay(1000);
    InitQSPI();

    InitSynthParams();
    SynthInit(samplerate, blocksize);
    EffectsInit(samplerate);
    MidiInit();

    InitSlots();
    InitSX1509Extenders();
    SetPage(MAIN_PAGE);

    hw.StartAudio(AudioCallback);

    Timer500ms();
    System::Delay(10);

    while (1)
    {
        ProcessButtons();
        ProcessEncoders();
        UpdateEncodersParams();
        UpdatePage();

        sx1509_leds.WritePin(6, midi_note_led);
    }
}

void ProcessButtons()
{
    bool any_button_change = sx1509_buttons.ReadAllPins();
    bool shift_pressed = sx1509_buttons.IsPressed(BUTTON_SHIFT);

    UpdateEncoderSwitches();

    if (any_button_change)
    {

        if (currentPage == MenuPage::FX_PAGE)
        {
            if (shift_pressed)
            {
                if (sx1509_buttons.isFallingEdge(ENC_1_SW))
                {
                    effectSlot[0].isActive = !effectSlot[0].isActive;
                    page_need_update = true;
                }
                if (sx1509_buttons.isFallingEdge(ENC_4_SW))
                {
                    effectSlot[1].isActive = !effectSlot[1].isActive;
                    page_need_update = true;
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
                    menu_slots[i].isEditMode = !menu_slots[i].isEditMode;
                    if (!menu_slots[i].isEditMode)
                    {
                        InitOneParamBlock(i, menu_slots[i].target_param, WHITE, BLACK);
                    }
                }
            }
        }

        if (shift_pressed)
        {
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_1))
            {
                bool osc_active_1 = paramManager.GetBool(P::OSC_ACTIVE_1);
                paramManager.SetBool(P::OSC_ACTIVE_1, !osc_active_1);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_2))
            {
                bool osc_active_2 = paramManager.GetBool(P::OSC_ACTIVE_2);
                paramManager.SetBool(P::OSC_ACTIVE_2, !osc_active_2);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_3))
            {
                bool osc_active_3 = paramManager.GetBool(P::OSC_ACTIVE_3);
                paramManager.SetBool(P::OSC_ACTIVE_3, !osc_active_3);
                UpdateLeds();
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_STORE))
            {
                update_for_preset_needed = true;
                isStoreMode = true;
                ResetPreset(currentPreset.number);
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
                SetPage(MenuPage::MTX_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_STORE))
            {
                update_for_preset_needed = true;
                isStoreMode = true;
                SavePreset(currentPreset.number, currentPreset);
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
                menu_slots[i].need_update = true;
            }
        }
        break;
    case FX_PAGE:
        if (encoderIncs[0] != 0)
        {
            effectSlot[0].need_update = true;
        }
        if (encoderIncs[3] != 0)
        {
            effectSlot[1].need_update = true;
        }
        break;
    default:
        for (size_t i = 0; i < 4; i++)
        { // Only 4 encoders
            if (encoderIncs[i] != 0)
            {
                uint8_t paramIndex = GetActiveParamIndex(i); // Get index of active parameter
                slots[paramIndex].need_update = true;
            }
        }
        break;
    }
}

void Callback(void *data)
{
    isBlink = !isBlink;
    blinkStateChanged = true;
    CpuUsageDisplay();
}

void Timer500ms()
{
    TimerHandle::Config tim_cfg;

    tim_cfg.periph = TimerHandle::Config::Peripheral::TIM_5;
    tim_cfg.enable_irq = true;

    auto tim_target_freq = 1;
    auto tim_base_freq = System::GetPClk2Freq();
    tim_cfg.period = tim_base_freq / tim_target_freq;

    tim_display.Init(tim_cfg);
    tim_display.SetCallback(Callback);
    tim_display.Start();

    System::Delay(10);
}

void CpuUsageDisplay(bool on)
{

    // float cpu_avg_load = cpu_load.GetAvgCpuLoad() * 100;
    // UartPrintf("CPU load: ", cpu_avg_load);

    if (on)
    {
        if (currentPage == MAIN_PAGE)
        {
            Paint_NewImage(cpu_load_block_data.data, 24, 24, 0, BLACK);
            Paint_Clear(BLACK);
            float cpu_avg_load = cpu_load.GetAvgCpuLoad() * 100;
            Paint_NumCentered(cpu_avg_load, 0, 24, 0, 1, Font8, WHITE, BLACK);
            OLED_Part_Transmit_DMA(&cpu_load_block_data, 104, 0, 128, 24);
            // UartPrint("CPU load: ", cpu_avg_load);
        }
    }
    else
    {
        Paint_NewImage(cpu_load_block_data.data, 24, 24, 0, BLACK);
        Paint_Clear(BLACK);
        OLED_Part_Transmit_DMA(&cpu_load_block_data, 104, 0, 128, 24);
    }
}
