#ifndef GLOBALS_H
#define GLOBALS_H

#include "log_uart.h"

#define VOICE_NUM 5 

extern int encoderIncs[5];
extern bool shift_pressed;
extern bool isVoiceActive[VOICE_NUM];
extern bool isOscSyncNeeded[OSC_NUM];
extern float osc_data_prev[OSC_NUM];
extern int isModAffectsOscFreq;

#endif