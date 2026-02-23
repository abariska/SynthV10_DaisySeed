#ifndef SYNTH_V9_H
#define SYNTH_V9_H

#include "parameters.h"
#include "sx1509_expander.h"
#include "voice.h"
#include "midi_handler.h"
#include "menu.h"

enum ProcessType {
    PROCESS_CONTROLS,
    UPDATE_PARAMS,
    PROCESS_DISPLAY
};

extern float scope_data[128]; 
extern int scope_data_index;
extern bool scope_data_ready;

void Timer500ms();
void Timer1ms();
void InitImages();
void DrawIntroPage();
void UpdateEncoderSwitches();
void UpdateEncodersParams();

#endif