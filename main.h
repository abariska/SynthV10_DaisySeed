#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "daisy.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include "daisysp-lgpl.h"
#include "parameters.h"
#include "sx1509_expander.h"
#include "voice.h"
#include "effects.h"
#include "midi_handler.h"
#include "daisy_core.h"
#include "display.h"
#include "menu.h"
#include "log_uart.h"

using namespace daisy;
using namespace daisysp;

enum ProcessType {
    PROCESS_CONTROLS,
    UPDATE_PARAMS,
    PROCESS_DISPLAY,
    COUNT_PROCESS_TYPES
};


extern MenuPage currentPage;

extern CpuLoadMeter cpu_load;
extern int encoderIncs[5];
extern SX1509 sx1509_buttons;
extern SX1509 sx1509_encoders;
extern SX1509 sx1509_leds;
extern Preset currentPreset;
extern float scope_data[128]; 
extern int scope_data_index;
extern bool scope_data_ready;

extern UartHandler uart_serial;
extern ParameterManager paramManager;

void Timer500ms();
void Timer1ms();
void ProcessButtons();
void ProcessEncoders();
void InitImages();
void DrawIntroPage();
void UpdateEncoderSwitches();
void UpdateEncodersParams();
void InitParamBlocks();
void CpuUsageDisplay();

#endif