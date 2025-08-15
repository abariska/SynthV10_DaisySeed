#include "main.h"
#include "daisy.h"
#include "sx1509_expander.h"
#include "midi_handler.h"
#include "oscillator.h"
#include "display.h"


using namespace daisy;

DaisySeed hw;

TimerHandle tim_display;
CpuLoadMeter cpu_load;

int encoderIncs[4];
int test = 0;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in, 
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    cpu_load.OnBlockStart();

    midiUart.Listen();
    midiUsb.Listen();

    while(midiUsb.HasEvents()) {
        auto msg = midiUsb.PopEvent();
        HandleMidiMessage(msg);
    }

    while(midiUart.HasEvents()) {
        auto msg = midiUart.PopEvent();
        HandleMidiMessage(msg);
    }
    
    for(size_t i = 0; i < size; i += 2)
    {
        float sig_after_fxL = 0.0f;
        float sig_after_fxR = 0.0f;

        // VoiceProcessTest(sig_after_fxL, sig_after_fxR);
        VoiceProcess(sig_after_fxL, sig_after_fxR);

        // for (size_t i = 0; i < 2; i++) {
        //     ProcessEffects(effectSlot[i], mix, sig_after_fxL, sig_after_fxR);
        // }
        // out[i] = sig_after_fxL;
        // out[i + 1] = sig_after_fxR;
        out[i] = sig_after_fxL;
        out[i + 1] = sig_after_fxR;
    }
    cpu_load.OnBlockEnd();  
}

int main(void)
{
    float samplerate;
    int blocksize = 4;

    hw.Configure();
    hw.Init();
    System::Delay(100);
    // hw.StartLog(true);  // Вимкнути для автономної роботи
    hw.SetAudioBlockSize(blocksize);
    samplerate = hw.AudioSampleRate(); 
    cpu_load.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

    OLED_1in5_Init();
    InitImages();
    DrawIntroPage();

    VoiceInit(samplerate, blocksize);
    InitSynthParams();
    EffectsInit(samplerate);
    
    InitLfo(samplerate);
    MidiInit();
    
    hw.DelayMs(1000);

    // DrawIntroPage2();
    // hw.DelayMs(1000);

    hw.StartAudio(AudioCallback);
    
    InitSlots();
    InitSX1509Extenders(); 
    SetPage(MAIN_PAGE);
    
    Timer500ms();

    while (1)
    {
        ProcessButtons();
        ProcessEncoders();
        UpdateEncodersParams();

        sx1509_leds.WritePin(6, midi_note_led);
}
}

void ProcessButtons() {
    bool any_button_change = sx1509_buttons.ReadAllPins();
    bool shift_pressed = sx1509_buttons.IsPressed(BUTTON_SHIFT);

    UpdateEncoderSwitches(); 

    if (any_button_change) {
        
        if (currentPage == MenuPage::FX_PAGE) {
            if (shift_pressed) {  
                if (sx1509_buttons.isFallingEdge(ENC_1_SW)) {
                    effectSlot[0].isActive = !effectSlot[0].isActive;
                }
                if (sx1509_buttons.isFallingEdge(ENC_4_SW)) {
                    effectSlot[1].isActive = !effectSlot[1].isActive;
                }
            } else {
                if (sx1509_buttons.isFallingEdge(ENC_1_SW)) {
                    SelectEffectPage(0);
                }
                if (sx1509_buttons.isFallingEdge(ENC_4_SW)) {
                    SelectEffectPage(1);
                }
            }
        } else if (currentPage == MenuPage::MAIN_PAGE) {
            for (size_t i = 0; i < 4; i++) {  // Тільки 4 енкодери
                if (sx1509_buttons.isFallingEdge(ENC_1_SW + i)) {
                    menu_slots[i].isEditMode = !menu_slots[i].isEditMode;
                    InitOneParamBlock(i, *allParams[menu_slots[i].assignedParam].target_param, 
                        allParams[menu_slots[i].assignedParam].label, WHITE, BLACK);
                }
            }
        }

        if (shift_pressed) {
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_1)) {
                params.osc[0].active = !params.osc[0].active;
                sx1509_leds.WritePin(LED_OSC_1, params.osc[0].active);
                Paint_NewImage(osc_on_block_data.data, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT, 0, BLACK);
                Paint_DrawString_EN(110, 0,params.osc[0].active ? "On" : "Off", &Font8, WHITE, BLACK);
                OLED_Part_Transmit_DMA(&osc_on_block_data, 0, 0, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT);

            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_2)) {
                params.osc[1].active = !params.osc[1].active;
                sx1509_leds.WritePin(LED_OSC_2, params.osc[1].active);
                Paint_NewImage(osc_on_block_data.data, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT, 0, BLACK);
                Paint_DrawString_EN(110, 0,params.osc[1].active ? "On" : "Off", &Font8, WHITE, BLACK);
                OLED_Part_Transmit_DMA(&osc_on_block_data, 32, 0, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_3)) {
                params.osc[2].active = !params.osc[2].active;
                sx1509_leds.WritePin(LED_OSC_3, params.osc[2].active);      
                Paint_NewImage(osc_on_block_data.data, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT, 0, BLACK);
                Paint_DrawString_EN(110, 0,params.osc[2].active ? "On" : "Off", &Font8, WHITE, BLACK);
                OLED_Part_Transmit_DMA(&osc_on_block_data, 64, 0, OSC_ON_BLOCK_WIDTH, OSC_ON_BLOCK_HEIGHT);
            }
        } else {
            if (sx1509_buttons.isFallingEdge(BUTTON_BACK)) {
                SetPage(MenuPage::MAIN_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_1)) {
                if (currentPage == MenuPage::OSCILLATOR_1_PAGE) {
                    ToggleActiveRow();  // Перемикання між рядами на тій же сторінці
                } else {
                    SetPage(MenuPage::OSCILLATOR_1_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_2)) {
                if (currentPage == MenuPage::OSCILLATOR_2_PAGE) {
                    ToggleActiveRow();
                } else {
                    SetPage(MenuPage::OSCILLATOR_2_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_OSC_3)) {
                if (currentPage == MenuPage::OSCILLATOR_3_PAGE) {
                    ToggleActiveRow();
                } else {
                    SetPage(MenuPage::OSCILLATOR_3_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_FLT)) {
                if (currentPage == MenuPage::FILTER_PAGE) {
                    ToggleActiveRow();
                } else {
                    SetPage(MenuPage::FILTER_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_AMP)) {
                if (currentPage == MenuPage::AMPLIFIER_PAGE) {
                    ToggleActiveRow();
                } else {
                    SetPage(MenuPage::AMPLIFIER_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_FX)) {
                SetPage(MenuPage::FX_PAGE);
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_LFO)) {
                if (currentPage == MenuPage::LFO_PAGE) {
                    ToggleActiveRow();
                } else {
                    SetPage(MenuPage::LFO_PAGE);
                }
            }
            if (sx1509_buttons.isFallingEdge(BUTTON_MTX)) {
                SetPage(MenuPage::MTX_PAGE);
            }
        }
    }
}
void ProcessEncoders(){
    
    bool any_encoder_change = sx1509_encoders.ReadAllPins();

    if (any_encoder_change) {
        encoderIncs[0] = EncoderInc(0, ENC_1_A, ENC_1_B);  
        encoderIncs[1] = EncoderInc(1, ENC_2_A, ENC_2_B);  
        encoderIncs[2] = EncoderInc(2, ENC_3_A, ENC_3_B);  
        encoderIncs[3] = EncoderInc(3, ENC_4_A, ENC_4_B);  

        test += encoderIncs[0];
    hw.Print("test: ", test);
    }
    

    if (currentPage == MAIN_PAGE) {
        for (size_t i = 0; i < NUM_ENCODERS; i++) {  // Тільки 4 енкодери
            if (encoderIncs[i] != 0) {
                menu_slots[i].need_update = true;
            }
        }
    } else {
        for (size_t i = 0; i < 4; i++) {  // Тільки 4 енкодери
            if (encoderIncs[i] != 0) {
                uint8_t paramIndex = GetActiveParamIndex(i);  // Отримуємо індекс активного параметра
                    slots[paramIndex].need_update = true;
            }
        }
    }
}

void Callback(void* data)
{
    isBlink = !isBlink;
    blinkStateChanged = true; 
    CpuUsageDisplay();
}

void Timer500ms() {
    TimerHandle::Config tim_cfg;

    tim_cfg.periph     = TimerHandle::Config::Peripheral::TIM_5;
    tim_cfg.enable_irq = true;

    auto tim_target_freq = 1;
    auto tim_base_freq   = System::GetPClk2Freq();
    tim_cfg.period       = tim_base_freq / tim_target_freq;

    tim_display.Init(tim_cfg);
    tim_display.SetCallback(Callback);
    tim_display.Start();
}

void SelectEffectPage(uint8_t slot){
        EffectName effect_to_show = effectSlot[slot].selectedEffect;
        switch (effect_to_show) {
            case EFFECT_OVERDRIVE:
                currentPage = MenuPage::OVERDRIVE_PAGE;
                break;
            case EFFECT_CHORUS:
                currentPage = MenuPage::CHORUS_PAGE;
                break;
            case EFFECT_COMPRESSOR:
                currentPage = MenuPage::COMPRESSOR_PAGE;
                break;
            case EFFECT_REVERB:
                currentPage = MenuPage::REVERB_PAGE;
                break;
            case EFFECT_NONE:
                break;
    }
}

void CpuUsageDisplay(bool on){
    
    if (on) {
        if (currentPage == MAIN_PAGE) {
            Paint_NewImage(cpu_load_block_data.data, 24, 24, 0, BLACK);
            Paint_Clear(BLACK);
            float cpu_avg_load = cpu_load.GetAvgCpuLoad() * 100;
            Paint_NumCentered(cpu_avg_load, 0, 24, 0, 1, Font8, WHITE, BLACK);
            OLED_Part_Transmit_DMA(&cpu_load_block_data, 104, 0, 128, 24);
        }
    } else {
        Paint_NewImage(cpu_load_block_data.data, 24, 24, 0, BLACK);
        Paint_Clear(BLACK);
        OLED_Part_Transmit_DMA(&cpu_load_block_data, 104, 0, 128, 24);
    }
}