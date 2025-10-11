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

// std::array<Osc, OSC_NUM * VOICE_NUM> osc;
// Adsr adsrMain[VOICE_NUM];
Adsr adsrMod;
MoogLadder flt;
Osc lfo;
Overdrive fltDrive;
// Random rnd[OSC_NUM * VOICE_NUM];
ModMatrix modMatrix[MOD_MATRIX_NUM];
Voice voice[VOICE_NUM];

float midiNoteToFreqTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];
float freqModTable[FREQ_MOD_TABLE_SIZE];

uint8_t noteNum = 60;
float frequency = 0;
// float phaseOffsets[OSC_NUM * VOICE_NUM];
// float pitch_correction[OSC_NUM * VOICE_NUM];
// float detune_correction[OSC_NUM * VOICE_NUM];
// float final_freq[OSC_NUM * VOICE_NUM];
float voice_velocity[VOICE_NUM] = {1.0f};
bool is_any_voice_active = false;
int voiceId = 0;
float filter_drive = 0.0f;

bool gate = false;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    for (size_t i = 0; i < VOICE_NUM; i++)
    {
        for (size_t j = 0; j < OSC_NUM; j++)
        {
            voice[i].osc[j].Init(samplerate);
            voice[i].rnd[j].Init();
        }
        voice[i].adsr.Init(samplerate, blocksize);
        
    }
    flt.Init(samplerate);
    adsrMod.Init(samplerate, blocksize);
    lfo.Init(samplerate);
    EffectsInit(samplerate);
    fltDrive.Init();
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
        if (voice[v].active && voice[v].timestamp < voice[oldestVoice].timestamp)
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
        // check if there is an inactive voice
        for(int v=0; v<VOICE_NUM; ++v)
            if(!voice[v].active) return v;

        // if there is no inactive voice, find the oldest voice and kill it
        int victim = FindOldestVoice();
        voice[victim].gate = false;
        voice[victim].active = false;
        return victim;
    }
    else
    {
        return 0;
    }
}

void HandleNoteOn(uint8_t note_in, uint8_t velocity)
{
    int v = AllocVoice();
    voice[v].active = true;
    voice[v].note = note_in;
    voice[v].freq = midiNoteToFreqTable[note_in];
    voice[v].vel = velocity / 127.0f;
    voice[v].gate = true;
    voice[v].timestamp = System::GetNow();
    
    if (!paramManager.GetBool(P::GLOBAL_LEGATO))
    {
        voice[v].adsr.Retrigger(false);
        adsrMod.Retrigger(false);
    }
    gate = true;
}

void HandleNoteOff(uint8_t note_in)
{
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voice[v].active && voice[v].note == note_in)
        {
            voice[v].active = false;
            voice[v].gate = false;
            break;
        }
    }

    is_any_voice_active = false;
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voice[v].active && voice[v].gate)
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
        // // if the voice is not active, skip it
        // if (!voiceState[v].active) continue;

        const float voiceFreq = voice[v].freq;
        const float voiceVel = voice[v].vel;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            float pitch = GetPitchTableValue(paramManager.GetValue(OSC_PITCH[oscId]));
            float detune = GetDetuneTableValue(paramManager.GetValue(OSC_DETUNE[oscId]));
            voice[v].final_freq[oscId] = voiceFreq * pitch * detune * pitch_bend_multiplier;
            paramManager.SetValue(OSC_FREQ[oscId], voice[v].final_freq[oscId]);
            float amp = paramManager.GetValue(OSC_AMP[oscId]) * voiceVel;
            float pw = paramManager.GetValue(OSC_PWM[oscId]);
            int waveform = static_cast<int>(paramManager.GetValue(OSC_WAVEFORM[oscId]));

            voice[v].osc[oscId].SetFreq(paramManager.GetValue(OSC_FREQ[oscId]));    
            voice[v].osc[oscId].SetAmp(amp);
            voice[v].osc[oscId].SetWaveform(waveform);
            voice[v].osc[oscId].SetPw(pw);
        }
        voice[v].adsr.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
        voice[v].adsr.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
        voice[v].adsr.SetSustainLevel(paramManager.GetValue(P::ADSR_SUSTAIN));
        voice[v].adsr.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));
    }

    flt.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    flt.SetRes(paramManager.GetValue(P::FILTER_RESONANCE));
    fltDrive.SetDrive(paramManager.GetValue(P::FILTER_DRIVE));
}

void VoiceProcess(float &voice_sig)
{
    voice_sig = 0.0f;
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        float voiceMix = 0.0f;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            voice[v].osc[oscId].PhaseProcess();

            if (paramManager.GetValue(OSC_ACTIVE[oscId]))
            {
                voiceMix += voice[v].osc[oscId].Process();
            }
        }

        float env = voice[v].adsr.Process(voice[v].gate);
        voice_sig += voiceMix * env / VOICE_NUM;

        // if the voice is not active and the envelope is below 0.00001f, kill the voice
        if (!(voice[v].active && voice[v].gate) && env <= 0.00001f) 
        {
            voice[v].gate = false;
            voice[v].active = false;
        }
    }
    float drive = fltDrive.Process(voice_sig);
    voice_sig = drive + (voice_sig * (1.0f - paramManager.GetValue(P::FILTER_DRIVE)));
    voice_sig = flt.Process(voice_sig);
    voice_sig = daisysp::fclamp(voice_sig, -1.0f, 1.0f);
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

    for (int i = 0; i < FREQ_MOD_TABLE_SIZE; i++)
    {
        freqModTable[i] = powf(2.0f, i / 12.0f);
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

float GetFreqModTableValue(int index)
{
    return freqModTable[index];
}