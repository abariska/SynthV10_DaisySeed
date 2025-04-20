#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "Parameters.h"
#include "Mcp23017.h"
#include "Button.h"
#include "Encoder_mcp.h"
#include "Voice.h"
#include "DisplayOLED.h"
#include "Menu.h"
#include "Effects.h"
#include "MidiHandler.h"
#include "daisy_core.h"


using namespace daisy;
using namespace daisysp;

#define LED_OSC_1  seed::D28
#define LED_OSC_2  seed::D27
#define LED_OSC_3  seed::D26    
#define LED_FX_1   seed::D25
#define LED_FX_2   seed::D24
#define LED_MIDI   seed::D23
#define VOICE_1  seed::D22
#define VOICE_2  seed::D21
#define VOICE_3  seed::D20
#define VOICE_4  seed::D19

// Global objects
extern OledDisplay<SSD130x4WireSpi128x64Driver> display;
extern VoiceUnit voice[NUM_VOICES];
extern MidiUsbHandler midi;
extern CpuLoadMeter cpu_load;
extern SynthParams params;
int encoderIncs[4];

Button_mcp button_osc_1(mcp_1, BUTTON_OSC_1, true);
Button_mcp button_osc_2(mcp_1, BUTTON_OSC_2, true);
Button_mcp button_osc_3(mcp_1, BUTTON_OSC_3, true);
Button_mcp button_flt(mcp_1, BUTTON_FLT, true);
Button_mcp button_amp(mcp_1, BUTTON_AMP, true);
Button_mcp button_fx(mcp_1, BUTTON_FX, true);
Button_mcp button_lfo(mcp_1, BUTTON_LFO, true);
Button_mcp button_mtx(mcp_1, BUTTON_MTX, true);
Button_mcp button_shift(mcp_1, BUTTON_SHIFT, true);
Button_mcp button_back(mcp_1, BUTTON_BACK, true);
Button_mcp sw_encoder_1(mcp_2, ENC_1_SW, true);
Button_mcp sw_encoder_2(mcp_2, ENC_2_SW, true);
Button_mcp sw_encoder_3(mcp_2, ENC_3_SW, true);
Button_mcp sw_encoder_4(mcp_2, ENC_4_SW, true);

GPIO led_osc_1;
GPIO led_osc_2;
GPIO led_osc_3;
GPIO led_fx_1;
GPIO led_fx_2;
GPIO led_midi;
GPIO led_out;
GPIO voice_1;
GPIO voice_2;
GPIO voice_3;

void InitLeds(){
    led_osc_1.Init(LED_OSC_1, GPIO::Mode::OUTPUT);
    led_osc_2.Init(LED_OSC_2, GPIO::Mode::OUTPUT);
    led_osc_3.Init(LED_OSC_3, GPIO::Mode::OUTPUT);
    led_fx_1.Init(LED_FX_1, GPIO::Mode::OUTPUT);
    led_fx_2.Init(LED_FX_2, GPIO::Mode::OUTPUT);
    led_midi.Init(LED_MIDI, GPIO::Mode::OUTPUT);
    voice_1.Init(VOICE_1, GPIO::Mode::OUTPUT);
    voice_2.Init(VOICE_2, GPIO::Mode::OUTPUT);
    voice_3.Init(VOICE_3, GPIO::Mode::OUTPUT);
}

// Function prototypes
void DisplayView(void* data);
void TimerDisplay();
void ProcessButtons();
void ProcessLeds();
void UpdateEncoders();
void UpdateButtons(){
    unsigned long currentTime = System::GetTick();
    button_back.Update(currentTime);
    button_osc_1.Update(currentTime);
    button_osc_2.Update(currentTime);
    button_osc_3.Update(currentTime);
    button_flt.Update(currentTime);
    button_amp.Update(currentTime);
    button_fx.Update(currentTime);
    button_lfo.Update(currentTime);
    button_mtx.Update(currentTime);
    button_shift.Update(currentTime);
    button_back.Update(currentTime);
    sw_encoder_1.Update(currentTime);
    sw_encoder_2.Update(currentTime);
    sw_encoder_3.Update(currentTime);
    sw_encoder_4.Update(currentTime);
}

#endif