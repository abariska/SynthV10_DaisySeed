#include "SynthV10.h"
#include "daisy.h"
using namespace daisy;

DaisySeed hw;

TimerHandle tim_display;
MidiUsbHandler midi;
CpuLoadMeter cpu_load;
extern Mcp23017 mcp_1;
extern Mcp23017 mcp_2;
extern SynthParams params;

int encoderIncs[4];

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    cpu_load.OnBlockStart();
    for(size_t i = 0; i < size; i += 2)
    {
        float sig_after_fxL, sig_after_fxR;
        float mix = 0.0f;

        for (size_t i = 0; i < NUM_VOICES; i++) {
            mix += voice[i].Process();
        }   
        mix /= NUM_VOICES;

        for (size_t i = 0; i < 2; i++) {
            ProcessEffects(effectSlot[i], mix, sig_after_fxL, sig_after_fxR);
        }
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
    // hw.StartLog(true);
    hw.SetAudioBlockSize(blocksize);
    samplerate = hw.AudioSampleRate(); 
    cpu_load.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

    for (size_t i = 0; i < NUM_VOICES; i++) {
        voice[i].Init(samplerate, blocksize);
    };
    EffectsInit(samplerate);
    InitSX1509Extenders(); 
    MidiInit();
    InitLfo(samplerate);
    InitSynthParams();
    InitImages();
    InitDisplayPages();
    DrawIntroPage();

    hw.DelayMs(1000);

    hw.StartAudio(AudioCallback);
    currentPage = EMPTY;
    SetPage(MAIN_PAGE);
    
    TimerDisplay();

    while (1)
    {
        ProcessButtons();
        ProcessEncoders();
        CheckEditParamOnMain();
        UpdateParamsWithEncoders();

        midi.Listen();
        while(midi.HasEvents())
        {
            auto msg = midi.PopEvent();
            HandleMidiMessage(msg);
            sx1509_leds.WritePin(LED_SHIFT, msg.AsNoteOn().note);
            sx1509_leds.WritePin(LED_BACK, !msg.AsNoteOff().note);
        }
        // CpuUsageDisplay();
}
}

void ProcessButtons() {

    if (sx1509_buttons.ReadAllPins()) {
        
        bool shift_pressed = sx1509_buttons.IsPressed(BUTTON_SHIFT);

        if (currentPage == MenuPage::FX_PAGE) {
            if (shift_pressed) {  
                if (sx1509_buttons.isRisingEdge(ENC_1_SW)) {
                effectSlot[0].isActive = !effectSlot[0].isActive;
                }
                if (sx1509_buttons.isRisingEdge(ENC_4_SW)) {
                    effectSlot[1].isActive = !effectSlot[1].isActive;
                }
            } else {
                if (sx1509_buttons.isRisingEdge(ENC_1_SW)) {
                    SelectEffectPage(0);
                }
                if (sx1509_buttons.isRisingEdge(ENC_4_SW)) {
                    SelectEffectPage(1);
                }
            }
        } 
        // else if (currentPage == MenuPage::MAIN_PAGE) {
        //     for (size_t i = 0; i < NUM_PARAM_BLOCKS; i++) {
        //         if (sw_encoder_1.RisingEdge()) {
        //             isParamEditMode[i] = !isParamEditMode[i];
        //         }
        //     }
        // }

        if (shift_pressed) {    
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_1)) {
                    params.voice.osc[0].active = !params.voice.osc[0].active;
                    sx1509_leds.WritePin(LED_OSC_1, params.voice.osc[0].active);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_2)) {
                    params.voice.osc[1].active = !params.voice.osc[1].active;
                    sx1509_leds.WritePin(LED_OSC_2, params.voice.osc[1].active);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_3)) {
                    params.voice.osc[2].active = !params.voice.osc[2].active;
                    sx1509_leds.WritePin(LED_OSC_3, params.voice.osc[2].active);
                }
        } else {
                if (sx1509_buttons.isRisingEdge(BUTTON_BACK)) {
                    SetPage(MenuPage::MAIN_PAGE);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_1)) {
                    SetPage(MenuPage::OSCILLATOR_1_PAGE);  
                    }
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_2)) {
                    SetPage(MenuPage::OSCILLATOR_2_PAGE);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_OSC_3)) {
                    SetPage(MenuPage::OSCILLATOR_3_PAGE);
                }   
                if (sx1509_buttons.isRisingEdge(BUTTON_FLT)) {
                    SetPage(MenuPage::FILTER_PAGE);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_AMP)) {
                    SetPage(MenuPage::AMPLIFIER_PAGE); 
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_FX)) {
                    SetPage(MenuPage::FX_PAGE);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_LFO)) {
                    SetPage(MenuPage::LFO_PAGE);
                }
                if (sx1509_buttons.isRisingEdge(BUTTON_MTX)) {
                    SetPage(MenuPage::MTX_PAGE);
                }
                // if (button_settings.RisingEdge()) {
                //     currentPage = MenuPage::SETTINGS_PAGE;
                // }
                // if (sw_encoder_1.RisingEdge()) {
                //     SelectEffectPage(0);
                // }
                // if (sw_encoder_2.RisingEdge()) {
                //     SelectEffectPage(1);
                // }
            }
    }
}

void ProcessEncoders(){
    
    if (sx1509_encoders.ReadAllPins()) {
        encoderIncs[0] = sx1509_encoders.EncoderInc(ENC_1_A, ENC_1_B);  
        encoderIncs[1] = sx1509_encoders.EncoderInc(ENC_2_A, ENC_2_B);  
        encoderIncs[2] = sx1509_encoders.EncoderInc(ENC_3_A, ENC_3_B);  
        encoderIncs[3] = sx1509_encoders.EncoderInc(ENC_4_A, ENC_4_B);  
    }
}

void DisplayView(void* data) {
    
}

void TimerDisplay() {
    TimerHandle::Config tim_cfg;

    /** TIM5 with IRQ enabled */
    tim_cfg.periph     = TimerHandle::Config::Peripheral::TIM_5;
    tim_cfg.enable_irq = true;

    /** Configure frequency (30Hz) */
    auto tim_target_freq = 30;
    auto tim_base_freq   = System::GetPClk2Freq();
    tim_cfg.period       = tim_base_freq / tim_target_freq;

    /** Initialize timer */
    tim_display.Init(tim_cfg);
    tim_display.SetCallback(DisplayView);

    /** Start the timer, and generate callbacks at the end of each period */
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

void CheckEditParamOnMain() {
    if (currentPage == MenuPage::MAIN_PAGE) {
        if (sx1509_buttons.isRisingEdge(ENC_1_SW)) {
            isParamEditMode[0] = !isParamEditMode[0];
            InitOneParamBlock(0, *allParams[slots[0].assignedParam].target_param, 
                allParams[slots[0].assignedParam].label, WHITE, BLACK);
        }
        if (sx1509_buttons.isRisingEdge(ENC_2_SW)) {
            isParamEditMode[1] = !isParamEditMode[1];
            InitOneParamBlock(1, *allParams[slots[1].assignedParam].target_param, 
                allParams[slots[1].assignedParam].label, WHITE, BLACK);
        }
        if (sx1509_buttons.isRisingEdge(ENC_3_SW)) {
            isParamEditMode[2] = !isParamEditMode[2];
            InitOneParamBlock(2, *allParams[slots[2].assignedParam].target_param, 
                allParams[slots[2].assignedParam].label, WHITE, BLACK);
        }
        if (sx1509_buttons.isRisingEdge(ENC_4_SW)) {
            isParamEditMode[3] = !isParamEditMode[3];
            InitOneParamBlock(3, *allParams[slots[3].assignedParam].target_param, 
                allParams[slots[3].assignedParam].label, WHITE, BLACK);
        }
    }
}