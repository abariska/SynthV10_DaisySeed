#include "main.h"
#include "daisy.h"
#include "sx1509_expander.h"
#include "midi_handler.h"
#include "oscillator.h"
#include "display.h"
#include "log_uart.h"
#include "parameters.h"
#include "display.h"
#include "globals.h"
#include "controls.h"

using namespace daisy;

using P = ParamUnitName;
using M = ModSource;

DaisySeed hw;
TimerHandle timer_500ms;
TimerHandle timer_1ms;
ProcessType process_type;

extern Preset currentPreset;

int test = 123;
float samplerate = 0;
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
static float scope_out = 0.0f;

void ProcessScope(float sig);

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size)
{
    cpu_load.OnBlockStart();

    UsbMidiProcess(); 
    UartMidiProcess();

    for (size_t i = 0; i < MOD_MATRIX_NUM; i++)
    {
        currentPreset.modMtx[i].RunMod();
        P modTarget = currentPreset.modMtx[i].GetModTarget();
        if (modTarget == P::OSC_FREQ_1 || modTarget == P::OSC_FREQ_2 || modTarget == P::OSC_FREQ_3)
        {
            static float oldModAmt = 0.0f;
            float modAmt = currentPreset.modMtx[i].GetModAmount();
            if (fabsf(modAmt - oldModAmt) > 0.000001f) {
                isModAffectsOscFreq++;
            } 
            oldModAmt = modAmt;
        }
    }

    for (size_t i = 0; i < size; i += 2)
    {
        float outL = 0.0f;
        float outR = 0.0f;
        float inL = in[i] * 0.5f;
        float inR = in[i+1] * 0.5f;

        ModSourcesProcess();
        VoiceProcess(outL, outR); 
        float fx1_outL = 0.0f;
        float fx1_outR = 0.0f;
        float fx2_outL = 0.0f;
        float fx2_outR = 0.0f;
        ProcessEffects(0, outL, outR, fx1_outL, fx1_outR);
        ProcessEffects(1, fx1_outL, fx1_outR, fx2_outL, fx2_outR);

        out[i] = (fx2_outL + inL) * cached_master_volume;
        out[i + 1] = (fx2_outR + inR) * cached_master_volume;
        // out[i] = outL;
        // out[i + 1] = outR;

        scope_out = outL + outR;
        ProcessScope(scope_out);
    }

    cpu_load.OnBlockEnd();
}

int main(void)
{
    int blocksize = 16;

    hw.Configure();
    hw.Init(true);
    UartSerialInit();

    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
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

    Timer500ms();
    Timer1ms();
    System::Delay(10);
    process_type = PROCESS_CONTROLS;

    hw.StartAudio(AudioCallback);

    while (1)
    {
        if (isMidiData)
        {
            DirtyFlagsToTrue();
            isMidiData = false;
        }
        switch (process_type)
        {
            case PROCESS_CONTROLS:
                ProcessEncoders();
                UpdateEncodersParams();
                break;
            case UPDATE_PARAMS:
                UpdateModSourcesParams();
                UpdateSynthParams();
                break;
            case PROCESS_DISPLAY:
                UpdatePage();
                break;
        }        
        if (update_1ms)
        {
            ProcessButtons();
            UpdateEncoderSwitches();
            // DrawScope();
            UpdatePWMLeds();
            // DrawVoicesBlock();
            UpdateVoiceLeds();

            update_1ms = false;
        }
        if (update_500ms)
        {
            DrawCpuUsage();
            UpdateStoreLed();
            update_500ms = false;
        }
        process_type = static_cast<ProcessType>((process_type + 1) % 3);
    }
}

void Callback500ms(void *data)
{
    isBlink = !isBlink;
    blinkStateChanged = true;
    update_500ms = true;
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

void ProcessScope(float sig)
{
    if (!scope_triggered && 
        scope_prev_sample <= scope_trigger_level && 
        sig > scope_trigger_level)
    {
        scope_triggered = true;
        scope_data_index = 0;
        scope_trigger_delay = 0;
    }
    
    if (scope_triggered)
    {
        if (scope_trigger_delay > 2)
        {
            scope_data[scope_data_index] = sig;
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
    
    scope_prev_sample = sig;
}


// void DrawVoicesBlock()
// {
//     Paint_NewImage(voices_block_data.data, VOICES_BLOCK_WIDTH, VOICES_BLOCK_HEIGHT, 0, BLACK);
//     Paint_Clear(BLACK);
    
//     static bool voice_active[VOICE_NUM] = {false};
//     static uint8_t voice_num = 1;
//     int x1 = 1, x2 = 1;
//     for (size_t i = 0; i < VOICE_NUM; i++)
//     {
//         if (voice_active[i] != voice[i].active) 
//         { 
//             voice_active[i] = voice[i].active; voice_num = voice_num + (voice[i].active ? 1 : -1); 
//             return; 
//         }
//         x2 = x1 + 6;
        
//         if (voice[i].active)
//         {
//             Paint_DrawRectangle(x1, 2, x2, 22, 0x08, DOT_PIXEL_1X1, DRAW_FILL_FULL);
//         } else {
//             Paint_DrawRectangle(x1, 2, x2, 22, 0x08, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//         }
//         x1 += 8;
//         voice_num += voice[i].active ? 1 : 0;
//     }
//     if (voice_num > 0) 
//     {
//         OLED_Transmit_DMA_Part(&voices_block_data, 0, 0, VOICES_BLOCK_WIDTH, VOICES_BLOCK_HEIGHT);
//         voice_num = 0; 
//     }
// }
