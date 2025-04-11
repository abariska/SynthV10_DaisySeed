#include "SynthV10.h"
#include "Encoder_mcp.h"
#include "daisy.h"
using namespace daisy;

DaisySeed hw;
Effects effects;
TimerHandle tim_display;
MidiUsbHandler midi;
CpuLoadMeter cpu_load;

int test_int = 0;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    cpu_load.OnBlockStart();
    for(size_t i = 0; i < size; i += 2)
    {
        // float sig_after_fxL, sig_after_fxR;
        float mix = 0.0f;

        if (params.global.isMono) {
            for (int i = 0; i < NUM_VOICES; i++) {
                mix += voice[i].Process();
            }
        } else {
            for (int i = 0; i < NUM_VOICES; i++) {
                if (voice[i].isVoiceActive) {
                    mix += voice[i].Process();
                }
            }
        }

        // effects.Process(synth.current_signal, synth.current_signal, &sig_after_fxL, &sig_after_fxR);

        out[i] = mix;
        out[i + 1] = mix;
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
    MidiInit();
    InitSynthParams();

    InitLfo(samplerate);
    for (int i = 0; i < NUM_VOICES; i++) {
        voice[i].Init(samplerate, blocksize);
    }
    effects.Init(samplerate);

    hw.StartAudio(AudioCallback);

    TimerDisplay();

    while (1)
    {
        // hw.PrintLine("Loop start %d", System::GetNow());
        mcp_1.Read();
        mcp_2.Read();
        ProcessButtons();
        ProcessEncoders();
        ProcessLeds(); 

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

void ProcessButtons() {
    
    UpdateButtons();

    bool shift_pressed = button_shift.IsPressed();

    if (button_osc_1.RisingEdge()) {
        currentPage = MenuPage::OSCILLATOR_1_PAGE;  
    }
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

    if (button_osc_2.RisingEdge()) {
        currentPage = MenuPage::OSCILLATOR_2_PAGE;
    }
    if (button_osc_3.RisingEdge()) {
        currentPage = MenuPage::OSCILLATOR_3_PAGE;
    }   
    if (button_flt.RisingEdge()) {
        currentPage = MenuPage::FILTER_PAGE;
    }
    if (button_amp.RisingEdge()) {
        currentPage = MenuPage::AMPLIFIER_PAGE; 
    }
    if (button_fx.RisingEdge()) {
        currentPage = MenuPage::FX_PAGE;
        params.effectUnits[0].isActive = !params.effectUnits[0].isActive;
    }
    if (button_lfo.RisingEdge()) {
        // currentPage = MenuPage::LFO_PAGE;
    }
    if (button_mtx.RisingEdge()) {
        currentPage = MenuPage::MTX_PAGE;
    }
    // if (button_settings.RisingEdge()) {
    //     currentPage = MenuPage::SETTINGS_PAGE;
    // }
    if (button_back.RisingEdge()) {
        currentPage = MenuPage::MAIN_PAGE;
    }
}

void ProcessEncoders() {
    
    UpdateEncoders();
    
    int encoder_inc_1 = encoder_1.Increment();
    int encoder_inc_2 = encoder_2.Increment();
    int encoder_inc_3 = encoder_3.Increment();
    int encoder_inc_4 = encoder_4.Increment();
    // int encoder_inc_dial = encoder_dial.Increment();

    switch (currentPage) {
        case MAIN_PAGE:
        params.voice.filter.cutoff = MapValue(
            params.voice.filter.cutoff,
            encoder_inc_1,
            50.0f,
            15000.0f
        );
        params.voice.filter.resonance = MapValue(
            params.voice.filter.resonance,
            encoder_inc_2,
            0.0f,
            1.0f
        );
        params.voice.adsr.attack = MapValue(
            params.voice.adsr.attack,
            encoder_inc_3,
            0.0f,
            1.0f
        );
        params.voice.adsr.decay = MapValue(
            params.voice.adsr.decay,
            encoder_inc_4,
            0.0f,
            1.0f
        );
        break;
        case OSCILLATOR_1_PAGE:
        params.voice.osc[0].waveform = MapValue(
            params.voice.osc[0].waveform,
            encoder_inc_1,
            0.0f,
            3.0f
        );
        params.voice.osc[0].pitch = MapValue(
            params.voice.osc[0].pitch,
            encoder_inc_2,
            -36,
            36
        );
        params.voice.osc[0].detune = MapValue(
            params.voice.osc[0].detune,
            encoder_inc_3,
            -0.50f,
            0.50f
        );
        params.voice.osc[0].amp = MapValue(
            params.voice.osc[0].amp,
            encoder_inc_4,
            0.0f,
            1.0f
        );
        break;
        case OSCILLATOR_2_PAGE:  
        params.voice.osc[1].waveform = MapValue(
            params.voice.osc[1].waveform,
            encoder_inc_1,
            0,
            4
        );
        params.voice.osc[1].pitch = MapValue(
            params.voice.osc[1].pitch,
            encoder_inc_2,
            -36,
            36
        );
        params.voice.osc[1].detune = MapValue(
            params.voice.osc[1].detune,
            encoder_inc_3,
            -0.50f,
            0.50f
        );
        params.voice.osc[1].amp = MapValue(
            params.voice.osc[1].amp,
            encoder_inc_4,
            0.0f,
            1.0f
        );
        break;
        case OSCILLATOR_3_PAGE:
        params.voice.osc[2].waveform = MapValue(
            params.voice.osc[2].waveform,
            encoder_inc_1,
            0,
            4
        );
        params.voice.osc[2].pitch = MapValue(
            params.voice.osc[2].pitch,
            encoder_inc_2,
            -36,
            36
        );
        params.voice.osc[2].detune = MapValue(
            params.voice.osc[2].detune,
            encoder_inc_3,
            -0.50f,
            0.50f
        );
        params.voice.osc[2].amp = MapValue(
            params.voice.osc[2].amp,
            encoder_inc_4,
            0.0f,
            1.0f
        );
        break;
        case AMPLIFIER_PAGE:
        params.voice.adsr.attack = MapValue(
            params.voice.adsr.attack,
            encoder_inc_1,
            0.0f,
            1.0f
        );
        params.voice.adsr.decay = MapValue(
            params.voice.adsr.decay,
            encoder_inc_2,
            0.0f,
            1.0f
        );
        params.voice.adsr.sustain = MapValue(
            params.voice.adsr.sustain,
            encoder_inc_3,
            0.0f,
            1.0f
        );
        params.voice.adsr.release = MapValue(
            params.voice.adsr.release,
            encoder_inc_4,
            0.0f,
            1.0f
        );
        break;
        case FILTER_PAGE:
        params.voice.filter.cutoff = MapValue(
            params.voice.filter.cutoff,
            encoder_inc_1,
            50.0f,
            15000.0f
        );
        params.voice.filter.resonance = MapValue(
            params.voice.filter.resonance,
            encoder_inc_2,
            0.0f,
            1.0f
        );
        break;
        case LFO_PAGE:
        params.lfo.waveform = MapValue(
            params.lfo.waveform,
            encoder_inc_1,
            0,
            3
        );
        params.lfo.freq = MapValue(
            params.lfo.freq,
            encoder_inc_2,
            0.0f,
            1.0f
        );
        params.lfo.amp = MapValue(
            params.lfo.amp,
            encoder_inc_3,
            0.0f,
            1.0f
        );

        break;
        case FX_PAGE:
            break;
        case OVERDRIVE_PAGE:
            params.effectUnits[0].overdrive.drive = MapValue(
                params.effectUnits[0].overdrive.drive,
                encoder_inc_1,
                0.0f,
                1.0f
            );
            break;
        case CHORUS_PAGE:
            params.effectUnits[0].chorus.lfoFreq = MapValue(
                params.effectUnits[0].chorus.lfoFreq,
                encoder_inc_1,
                0.0f,
                1.0f
            );
            params.effectUnits[0].chorus.lfoDepth = MapValue(
                params.effectUnits[0].chorus.lfoDepth,
                encoder_inc_2,
                0.0f,
                1.0f
            );
            params.effectUnits[0].chorus.feedback = MapValue(
                params.effectUnits[0].chorus.feedback,
                encoder_inc_3,
                0.0f,
                1.0f
            );
            params.effectUnits[0].chorus.pan = MapValue(
                params.effectUnits[0].chorus.pan,
                encoder_inc_4,
                0.0f,
                1.0f
            );
            break;
        case COMPRESSOR_PAGE:
            params.effectUnits[0].compressor.attack = MapValue(
                params.effectUnits[0].compressor.attack,
                encoder_inc_1,
                0.0f,
                1.0f
            );
            params.effectUnits[0].compressor.release = MapValue(
                params.effectUnits[0].compressor.release,
                encoder_inc_2,
                0.0f,
                1.0f
            );
            params.effectUnits[0].compressor.threshold = MapValue(
                params.effectUnits[0].compressor.threshold,
                encoder_inc_3,
                0.0f,
                1.0f
            );
            params.effectUnits[0].compressor.ratio = MapValue(
                params.effectUnits[0].compressor.ratio,
                encoder_inc_4,
                1.0f,
                10.0f
            );
            break;
        case REVERB_PAGE:
            params.effectUnits[0].reverb.dryWet = MapValue(
                params.effectUnits[0].reverb.dryWet,
                encoder_inc_1,
                0.0f,
                1.0f
            );
            params.effectUnits[0].reverb.feedback = MapValue(
                params.effectUnits[0].reverb.feedback,
                encoder_inc_2,
                0.0f,
                1.0f
            );
            params.effectUnits[0].reverb.lpFreq = MapValue(
                params.effectUnits[0].reverb.lpFreq,
                encoder_inc_3,
                0.0f,
                5000.0f
            );
            break;
        case MTX_PAGE:
            break;
        case SETTINGS_PAGE:
        break;
        case STORE_PAGE:
        break;
        case LOAD_PAGE:
        break;
        default:
        break;
    }
}

void ProcessLeds() {
    led_osc_1.SetState(params.voice.osc[0].active);
    led_osc_2.SetState(params.voice.osc[1].active);
    led_osc_3.SetState(params.voice.osc[2].active);
    led_fx.SetState(params.effectUnits[0].isActive || params.effectUnits[1].isActive);
    led_midi.SetState(midi.HasEvents());
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

