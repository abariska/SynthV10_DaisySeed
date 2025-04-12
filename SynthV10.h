#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "Parameters.h"
#include "Mcp23017.h"
#include "Led.h"
#include "Button.h"
#include "Encoder_mcp.h"
#include "Voice.h"
#include "DisplayOLED.h"
#include "Menu.h"
#include "Effects.h"
#include "MidiHandler.h"

using namespace daisy;
using namespace daisysp;

// Global objects
extern OledDisplay<SSD130x4WireSpi128x64Driver> display;
extern SynthParams params;
extern EffectUnitParams effectParams;
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

Led_mcp led_osc_1(mcp_1, LED_OSC_1, false);
Led_mcp led_osc_2(mcp_1, LED_OSC_2, false);
Led_mcp led_osc_3(mcp_1, LED_OSC_3, false);
Led_mcp led_fx(mcp_1, LED_FX, false);
Led_mcp led_midi(mcp_1, LED_MIDI, false);
Led_mcp led_out(mcp_1, LED_OUT, false);

Enc_mcp encoder_1(mcp_2, ENC_1_A, ENC_1_B, ENC_1_SW, true);
Enc_mcp encoder_2(mcp_2, ENC_2_A, ENC_2_B, ENC_2_SW, true);
Enc_mcp encoder_3(mcp_2, ENC_3_A, ENC_3_B, ENC_3_SW, true);
Enc_mcp encoder_4(mcp_2, ENC_4_A, ENC_4_B, ENC_4_SW, true);
Enc_mcp encoder_dial(mcp_2, ENC_DIAL_A, ENC_DIAL_B, ENC_DIAL_SW, true);

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

    enum LoopUpdate {
        MAIN_LOOP,
        BUTTON_LOOP,
        ENCODER_LOOP,
        LED_LOOP,
        DISPLAY_LOOP
    };

#endif