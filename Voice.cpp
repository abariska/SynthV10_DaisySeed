#include "voice.h"
#include "daisy_seed.h"
#include "oscillator.h"
#include "parameters.h"
#include "midi_handler.h"
#include "sx1509_expander.h"

#define DTCM __attribute__((section(".dtcmram_bss")))

using P = ParamUnitName;
using namespace daisy;
using M = ModSource;

std::array<Osc, OSC_NUM * VOICE_NUM> osc;
Adsr adsrMain[VOICE_NUM];
Adsr adsrMod;
MoogLadder flt;
Osc lfo;
Random rnd[OSC_NUM * VOICE_NUM];
ModMatrix modMatrix[MOD_MATRIX_NUM];
VoiceState voiceState[VOICE_NUM];

float midiNoteToFreqTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];

uint8_t noteNum = 60;
float frequency = 0;
float phaseOffsets[OSC_NUM * VOICE_NUM];
float pitch_correction[OSC_NUM * VOICE_NUM];
float detune_correction[OSC_NUM * VOICE_NUM];
float final_freq[OSC_NUM * VOICE_NUM];
float voice_velocity[VOICE_NUM] = {1.0f};
bool is_any_voice_active = false;
int voiceId = 0;

bool gate = false;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    for (size_t i = 0; i < OSC_NUM * VOICE_NUM; i++)
    {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        adsrMain[v].Init(samplerate, blocksize);
    }

    adsrMod.Init(samplerate, blocksize);

    for (size_t i = 0; i < OSC_NUM * VOICE_NUM; i++)
    {
        rnd[i].Init();
    }
    lfo.Init(samplerate);
    EffectsInit(samplerate);
}

void ModSourcesProcess()
{
    lfo.PhaseProcess();
    modulators[static_cast<int>(M::LFO)].value = lfo.Process() / 2.0f + 0.5f;
    modulators[static_cast<int>(M::ADSR)].value = adsrMod.Process(gate);

    modulators[static_cast<int>(M::MOD_WHEEL)].value = mod_wheel_value;
    modulators[static_cast<int>(M::AFTERTOUCH)].value = aftertouch_value;
}

void UpdateModSourcesParams()
{
    lfo.SetFreq(paramManager.GetValue(P::MOD_LFO_FREQ));
    lfo.SetWaveform(paramManager.GetValue(P::MOD_LFO_WAVEFORM));
    lfo.SetAmp(paramManager.GetValue(P::MOD_LFO_DEPTH));

    adsrMod.SetAttackTime(paramManager.GetValue(P::MOD_ADSR_ATTACK), 1.0f);
    adsrMod.SetDecayTime(paramManager.GetValue(P::MOD_ADSR_DECAY));
    adsrMod.SetSustainLevel(paramManager.GetValue(P::MOD_ADSR_SUSTAIN));
    adsrMod.SetReleaseTime(paramManager.GetValue(P::MOD_ADSR_RELEASE));

}

int FindOldestVoice()
{
    int oldestVoice = 0;
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voiceState[v].active && voiceState[v].timestamp < voiceState[oldestVoice].timestamp)
        {
            oldestVoice = v;
        }
    }
    return oldestVoice;
}

int AllocVoice()
{
    if (!paramManager.GetBool(P::GLOBAL_MONO))
    {
        voiceId = (voiceId + 1) % VOICE_NUM;
        bool found = false;
        for(int v=0; v<VOICE_NUM; ++v)
        {
            if(!voiceState[v].active)
            {
                found = true;
            }
        }
        if(!found || paramManager.GetBool(P::GLOBAL_MONO))
        {
            int victim = FindOldestVoice();
            voiceState[victim].gate = false;
            voiceState[victim].active = false; // залишаємо огинаючій відпасти
            return victim;
        }
        else
        {
            return voiceId;
        }
    }
    else
    {
        int victim = FindOldestVoice();
            voiceState[victim].gate = false;
            voiceState[victim].active = false; // залишаємо огинаючій відпасти
            return victim;
    }

}

void HandleNoteOn(uint8_t note_in, uint8_t velocity)
{
    int v = AllocVoice();
    voiceState[v].active = true;
    voiceState[v].note = note_in;
    voiceState[v].freq = midiNoteToFreqTable[note_in];
    voiceState[v].vel = velocity / 127.0f;
    voiceState[v].gate = true;
    voiceState[v].timestamp = System::GetNow();
    
    if (!paramManager.GetBool(P::GLOBAL_LEGATO) && !paramManager.GetBool(P::GLOBAL_MONO))
    {
        adsrMain[v].Retrigger(false);
        adsrMod.Retrigger(false);
    }
    gate = true;
}

void HandleNoteOff(uint8_t note_in)
{
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voiceState[v].active && voiceState[v].note == note_in)
        {
            voiceState[v].gate = false;
            voiceState[v].active = false;
            break;
        }
    }

    is_any_voice_active = false;
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voiceState[v].active && voiceState[v].gate)
        {
            is_any_voice_active = true;
            break;
        }
    }
    gate = is_any_voice_active;
}

void UpdateSynthParams()
{
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        const float voiceFreq = voiceState[v].freq;
        const float voiceVel = voiceState[v].vel;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            size_t idx = v * OSC_NUM + oscId;
            float pitch = GetPitchTableValue(paramManager.GetValue(OSC_PITCH[oscId]));
            float detune = GetDetuneTableValue(paramManager.GetValue(OSC_DETUNE[oscId]));
            float freq = voiceFreq * pitch * detune * pitch_bend_multiplier;
            float amp = paramManager.GetValue(OSC_AMP[oscId]) * voiceVel;
            float pw = paramManager.GetValue(OSC_PWM[oscId]);
            int waveform = static_cast<int>(paramManager.GetValue(OSC_WAVEFORM[oscId]));

            osc[idx].SetFreq(freq);
            osc[idx].SetAmp(amp);
            osc[idx].SetWaveform(waveform);
            osc[idx].SetPw(pw);
        }
        adsrMain[v].SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
        adsrMain[v].SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
        adsrMain[v].SetSustainLevel(paramManager.GetValue(P::ADSR_SUSTAIN));
        adsrMain[v].SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));
    }

    flt.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    flt.SetRes(paramManager.GetValue(P::FILTER_RESONANCE));
}

void VoiceProcess(float &voice_sig)
{
    voice_sig = 0.0f;
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        float voiceMix = 0.0f;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            size_t idx = v * OSC_NUM + oscId;
            osc[idx].PhaseProcess();

            if (paramManager.GetValue(OSC_ACTIVE[oscId]))
            {
                voiceMix += osc[idx].Process();
            }
        }

        voiceMix = daisysp::fclamp(voiceMix, -1.0f, 1.0f) / VOICE_NUM;

        float env = adsrMain[v].Process(voiceState[v].gate);

        voice_sig += voiceMix * env;

        if (!(voiceState[v].active && voiceState[v].gate) && env <= 0.00001f)
        {
            voiceState[v].gate = false;
        }
    }
    // voice_sig = daisysp::fclamp(voice_sig, -1.0f, 1.0f);

    voice_sig = flt.Process(voice_sig);
}



void InitPitchTables()
{
    for (int i = 0; i < 128; i++)
    {
        midiNoteToFreqTable[i] = 440.0f * powf(2.0f, (i - 69) / 12.0f);
    }

    for (int i = 0; i < PITCH_TABLE_SIZE; i++)
    {
        pitchTable[i] = powf(2.0f, (i - PITCH_CENTER_INDEX) / 12.0f);
    }
    
    for (int i = 0; i < DETUNE_TABLE_SIZE; i++)
    {
        detuneTable[i] = powf(2.0f, (i - DETUNE_CENTER_INDEX) / 1200.0f);
    }

    for (int i = 0; i < PITCH_BEND_TABLE_SIZE; i++)
    {
        pitchBendTable[i] = powf(2.0f, (i - PITCH_BEND_CENTER_INDEX) / 1200.0f);
    }
}

float midiNoteToFreq(int note)
{
    return midiNoteToFreqTable[note];
}

float GetPitchTableValue(int index)
{
    return pitchTable[index + PITCH_CENTER_INDEX];
}

float GetDetuneTableValue(int index)
{
    return detuneTable[index + DETUNE_CENTER_INDEX];
}

float GetPitchBendTableValue(int index)
{
    return pitchBendTable[index + PITCH_BEND_CENTER_INDEX];
}