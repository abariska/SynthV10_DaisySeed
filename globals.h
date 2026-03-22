#ifndef GLOBALS_H
#define GLOBALS_H

#define VOICE_NUM 5
#define OSC_NUM 3
#define PRESET_NUM 40
#define MOD_MATRIX_NUM 7

#define DTCM __attribute__((section(".dtcm_bss")))
// #define DTCM_DATA __attribute__((section(".dtcm_data")))
#define ITCM __attribute__((section(".itcm_text")))

struct Preset;

extern int encoderIncs[5];
extern bool shift_pressed;
extern bool isVoiceActive[VOICE_NUM];
extern bool isOscSyncNeeded[OSC_NUM];
extern float osc_data_prev[OSC_NUM];
extern int isModAffectsOscFreq;

extern Preset currentPreset;

#endif