#ifndef VOICE_H
#define VOICE_H

#include "parameters.h"
#include "globals.h"

#define SEMITONE_RATIO 1.0594630943592953f
#define CENT_RATIO 1.0005777895065549f
#define PITCH_TABLE_SIZE 73
#define PITCH_CENTER_INDEX 36
#define DETUNE_TABLE_SIZE 201
#define DETUNE_CENTER_INDEX 100
#define PITCH_BEND_TABLE_SIZE 401
#define PITCH_BEND_CENTER_INDEX 200
#define FREQ_MOD_TABLE_SIZE 101
#define PANNING_TABLE_SIZE 101

#define MAX_NOTE_STACK 10

using namespace daisy;
using namespace daisysp;

extern float samplerate;
extern float lfo_value;
extern float midiNoteToFreqTable[128];
extern float pitchTable[PITCH_TABLE_SIZE];
extern float detuneTable[DETUNE_TABLE_SIZE];
extern float pitchBendTable[PITCH_BEND_TABLE_SIZE];
extern bool isOscSyncNeeded[OSC_NUM];
extern OscLfo lfo;

struct GlobalCache
{
    float portamento = 0.0f;
    bool mono = false;
    bool legato = false;
    float pan = 0.0f;
    bool lfo_trigger = false;
    float master_volume = 0.1f;
};
extern GlobalCache cache_global;

struct OscCache
{
    int waveform;
    float pitch;
    float detune;
    float freq_factor;
    float amp;
    float pw;
    bool active;
};
extern OscCache cache_osc[OSC_NUM];

struct VoiceCache
{
    float base_freq;
    float base_amp;
    float pan_correction[2];
    float filter_cutoff;
    float filter_resonance;
    float adsr_attack;
    float adsr_decay;
    float adsr_sustain;
    float adsr_release;
};
extern VoiceCache cache_voice[VOICE_NUM];

struct ModCache
{
    float lfo_value = 1.0f;
    float adsr_value = 1.0f;
    float mod_wheel_value = 1.0f;
    float aftertouch_value = 1.0f;
};
extern ModCache cache_mod;

struct Voice
{
    Osc     osc[OSC_NUM];
    Adsr    adsr, adsrMod;
    LadderFilter flt;
    Random  rnd[OSC_NUM];
    float   phaseOffset;
    float   pitch_correction[OSC_NUM];
    float   detune_correction[OSC_NUM];
    float   final_freq[OSC_NUM];
    float   final_amp[OSC_NUM];
    float   final_pw[OSC_NUM];
    
    bool     active     = false;
    bool     gate       = false;
    int16_t  note       = -1;
    float    freq       = 0.0f;
    float    vel        = 1.0f;
    std::uint32_t timestamp  = 0;
};

extern Voice voice[VOICE_NUM];
extern AudioParamsDirty dirty;
extern Adsr adsrModGlobal;

void HandleNoteOn(uint8_t note_in, uint8_t velocity);
void HandleNoteOff(uint8_t note_in);
inline void HandlePitchBend(int16_t pitch_bend); 
void SynthInit(float samplerate, int blocksize);
void VoiceProcess(float &out_sigL, float &out_sigR);
void VoicePanningInit();
float softClip(float x);
void ModSourcesProcess();
void InitPitchTables();
float GetPitchTableValue(int index);
float GetDetuneTableValue(int index);
float GetPitchBendTableValue(int index);
float GetVelocityToAmpTableValue(uint8_t velocity);
void UpdateSynthParams();
void PushNote(uint8_t note);
uint8_t PopNote(uint8_t note);
extern float panningTable[101][2];
void InitPanningTable();
void SynthVoiceReset(uint8_t voice_num);
void ModMatrixReset(uint8_t mod_matrix_num);
#endif
