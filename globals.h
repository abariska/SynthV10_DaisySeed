#ifndef GLOBALS_H
#define GLOBALS_H

#define VOICE_NUM 5 
#define OSC_NUM 3

extern int encoderIncs[5];
extern bool shift_pressed;
extern bool isVoiceActive[VOICE_NUM];
extern bool isOscSyncNeeded[OSC_NUM];
extern float osc_data_prev[OSC_NUM];
extern int isModAffectsOscFreq;

extern Preset currentPreset;
extern MenuPage currentPage;

#endif