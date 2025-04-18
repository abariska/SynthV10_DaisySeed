#include "SynthV10.h"
#include "Encoder_mcp.h"
#include "daisy.h"
using namespace daisy;

DaisySeed hw;

TimerHandle tim_display;
MidiUsbHandler midi;
CpuLoadMeter cpu_load;
extern Mcp23017 mcp_1;
extern Mcp23017 mcp_2;
extern SynthParams params;

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
    hw.SetAudioBlockSize(blocksize);
    samplerate = hw.AudioSampleRate(); 
    cpu_load.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

    Display_Init();
    InitMcp();
    InitLeds();
    MidiInit();
    InitSynthParams();
    InitLfo(samplerate);
    EffectsInit(samplerate);

    AssignParamsForPage(currentPage); 
    UpdateParamsWithEncoders();

    hw.StartAudio(AudioCallback);

    TimerDisplay();

    while (1)
    {
        // hw.PrintLine("Loop start %d", System::GetNow());
        mcp_1.Read();
        mcp_2.Read();
        ProcessButtons();
        ProcessLeds(); 
        UpdateEncoders();
        UpdateParamsWithEncoders();

        // MIDI processing
        midi.Listen();
        while(midi.HasEvents())
        {
            auto msg = midi.PopEvent();
            HandleMidiMessage(msg);
    }
        // hw.PrintLine("Loop end %d", System::GetNow());
        // System::Delay(100);  // Add delay for better output

        // if (button_lfo.IsPressed()) {
        //     synth.HandleNoteOn(440.0f, 1.0f);
        // }
    }
}

void UpdateEncoders() {
    encoder_1.Debounce();
    encoder_2.Debounce();
    encoder_3.Debounce();
    encoder_4.Debounce();

    encoderIncs[0] = encoder_1.Increment();
    encoderIncs[1] = encoder_2.Increment();
    encoderIncs[2] = encoder_3.Increment();
    encoderIncs[3] = encoder_4.Increment();
}

void ProcessButtons() {
    
    UpdateButtons();

    bool shift_pressed = button_shift.IsPressed();

    
    if (shift_pressed) {    
        if (button_osc_1.RisingEdge()) {
            params.voice.osc[0].active = !params.voice.osc[0].active;
        }
        if (button_osc_2.RisingEdge()) {
            params.voice.osc[1].active = !params.voice.osc[1].active;
        }
        if (button_osc_3.RisingEdge()) {
            params.voice.osc[2].active = !params.voice.osc[2].active;
        }
    }
    if (button_osc_1.RisingEdge()) {
        SetPage(MenuPage::OSCILLATOR_1_PAGE);  
    }
    if (button_osc_2.RisingEdge()) {
        SetPage(MenuPage::OSCILLATOR_2_PAGE);
    }
    if (button_osc_3.RisingEdge()) {
        SetPage(MenuPage::OSCILLATOR_3_PAGE);
    }   
    if (button_flt.RisingEdge()) {
        SetPage(MenuPage::FILTER_PAGE);
    }
    if (button_amp.RisingEdge()) {
        SetPage(MenuPage::AMPLIFIER_PAGE); 
    }
    if (button_fx.RisingEdge()) {
        SetPage(MenuPage::FX_PAGE);
    }
    if (button_lfo.RisingEdge()) {
        currentPage = MenuPage::LFO_PAGE;
    }
    // if (button_mtx.RisingEdge()) {
    //     SetPage(MenuPage::MTX_PAGE);
    // }
    // if (button_settings.RisingEdge()) {
    //     currentPage = MenuPage::SETTINGS_PAGE;
    // }
    if (button_back.RisingEdge()) {
        currentPage = MenuPage::MAIN_PAGE;
    }
}

void ProcessLeds() {
    led_osc_1.Write(params.voice.osc[0].active);
    led_osc_2.Write(params.voice.osc[1].active);
    led_osc_3.Write(params.voice.osc[2].active);
    led_fx_1.Write(effectSlot[0].isActive);
    led_fx_2.Write(effectSlot[1].isActive);
    led_midi.Write(voice[1].isGated);
    led_out.Write(voice[2].isGated);
    voice_1.Write(voice[0].isGated);
    voice_2.Write(voice[1].isGated);
    voice_3.Write(voice[2].isGated);
}

void DisplayView(void* data) {
    DrawMenu();
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

