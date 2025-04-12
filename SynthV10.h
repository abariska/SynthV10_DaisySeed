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

#define LED_OSC_1  seed::D27
#define LED_OSC_2  seed::D26
#define LED_OSC_3  seed::D25    
#define LED_FX_1   seed::D24
#define LED_FX_2   seed::D23
#define LED_MIDI   seed::D22
#define VOICE_1  seed::D20
#define VOICE_2  seed::D19
#define VOICE_3  seed::D18
#define VOICE_4  seed::D17
#define VOICE_5  seed::D16
#define VOICE_6  seed::D15

// Global objects
extern OledDisplay<SSD130x4WireSpi128x64Driver> display;
extern Synth synth;
extern Effects effects;
extern VoiceUnit voice[NUM_VOICES];
extern MidiUsbHandler midi;
extern CpuLoadMeter cpu_load;

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

Enc_mcp encoder_1(mcp_2, ENC_1_A, ENC_1_B, ENC_1_SW, true);
Enc_mcp encoder_2(mcp_2, ENC_2_A, ENC_2_B, ENC_2_SW, true);
Enc_mcp encoder_3(mcp_2, ENC_3_A, ENC_3_B, ENC_3_SW, true);
Enc_mcp encoder_4(mcp_2, ENC_4_A, ENC_4_B, ENC_4_SW, true);
Enc_mcp encoder_dial(mcp_2, ENC_DIAL_A, ENC_DIAL_B, ENC_DIAL_SW, true);

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
GPIO voice_4;
GPIO voice_5;
GPIO voice_6;

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
    voice_4.Init(VOICE_4, GPIO::Mode::OUTPUT);
    voice_5.Init(VOICE_5, GPIO::Mode::OUTPUT);
    voice_6.Init(VOICE_6, GPIO::Mode::OUTPUT);
}

// Function prototypes
void DisplayView(void* data);
void TimerDisplay();
void ProcessButtons();
void ProcessEncoders();
void ProcessLeds();

void UpdateButtons(){
    button_back.Update(System::GetTick());
    button_osc_1.Update(System::GetTick());
    button_osc_2.Update(System::GetTick());
    button_osc_3.Update(System::GetTick());
    button_flt.Update(System::GetTick());
    button_amp.Update(System::GetTick());
    button_fx.Update(System::GetTick());
    button_lfo.Update(System::GetTick());
    button_mtx.Update(System::GetTick());
    button_shift.Update(System::GetTick());
}

void UpdateEncoders(){
    encoder_1.Debounce();
    encoder_2.Debounce();
    encoder_3.Debounce();
    encoder_4.Debounce();
    encoder_dial.Debounce();
}

float MapValue(float current_value, int increment, float minVal, float maxVal) {
    // Determine sensitivity based on value type
    float sensitivity;
    if (minVal == 0.0f && maxVal == 1.0f) {
        sensitivity = 0.01f;  // For normalized values (0-1)
    } else if (minVal == -1.0f && maxVal == 1.0f) {
        sensitivity = 0.01f;  // For values in range (-1,1)
    } else {
        sensitivity = 1.0f;   // For other values
    }

    float new_value = current_value + (increment * sensitivity);
    
    // Normalize value between 0 and 1
    if (new_value > maxVal) {
        new_value = maxVal;
    } else if (new_value < minVal) {
        new_value = minVal;
    }
    
    // Return value in original range
    return new_value;
}

#endif