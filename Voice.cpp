#include "voice.h"
#include "daisy_seed.h"
#include "oscillator.h"
#include "parameters.h"
#include "midi_handler.h"

#define DTCM __attribute__((section(".dtcmram_bss")))

using P = ParamUnitName;

using M = ModSource;

std::array<Osc, OSC_NUM> osc;
Adsr adsrMain;
Adsr adsrMod;
MoogLadder flt;
Oscillator lfo;
Random rnd[OSC_NUM];
ModMatrix modMatrix[MOD_MATRIX_NUM];

float midiNoteToFreqTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];

uint8_t noteNum = 60;
float frequency = 0;
float phaseOffsets[OSC_NUM];
float pitch_correction[OSC_NUM];
float detune_correction[OSC_NUM];
float final_freq[OSC_NUM];
float velocity_factor = 1.0f;

const int maxNotes = 16;
int activeNotes[maxNotes];
int activeNoteCount = 0;
bool gate = false;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    for (size_t i = 0; i < OSC_NUM; i++)
    {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);
    adsrMod.Init(samplerate, blocksize);
    for (size_t i = 0; i < OSC_NUM; i++)
    {
        rnd[i].Init();
    }
    lfo.Init(samplerate);
    EffectsInit(samplerate);
}

void ModSourcesProcess()
{
    lfo.SetFreq(paramManager.GetValue(P::MOD_LFO_FREQ));
    lfo.SetWaveform(paramManager.GetValue(P::MOD_LFO_WAVEFORM));
    lfo.SetAmp(paramManager.GetValue(P::MOD_LFO_DEPTH));
    modulators[static_cast<int>(M::LFO)].value = lfo.Process();

    adsrMod.SetAttackTime(paramManager.GetValue(P::MOD_ADSR_ATTACK), 1.0f);
    adsrMod.SetDecayTime(paramManager.GetValue(P::MOD_ADSR_DECAY));
    adsrMod.SetSustainLevel(paramManager.GetValue(P::MOD_ADSR_SUSTAIN));
    adsrMod.SetReleaseTime(paramManager.GetValue(P::MOD_ADSR_RELEASE));
    modulators[static_cast<int>(M::ADSR)].value = adsrMod.Process(gate);

    modulators[static_cast<int>(M::MOD_WHEEL)].value = mod_wheel_value;
    modulators[static_cast<int>(M::AFTERTOUCH)].value = aftertouch_value;
}

void HandleNoteOn(uint8_t note_in, uint8_t velocity)
{
    bool isNotesPlaying = (activeNoteCount > 0);
    // Add this note to the list of active notes
    if (activeNoteCount < maxNotes)
    {
        activeNotes[activeNoteCount] = note_in;
        activeNoteCount++;
        velocity_factor = velocity / 127.0f * 0.8f;
        noteNum = note_in;
        frequency = midiNoteToFreqTable[note_in];

        for (size_t i = 0; i < OSC_NUM; i++)
        {
            phaseOffsets[i] = rnd[i].GetFloat(0.0f, 0.000000001f);
            osc[i].SetPhaseOffset(phaseOffsets[i]);
        }

        if (!(isNotesPlaying && paramManager.GetBool(P::GLOBAL_LEGATO)))
        {
            adsrMain.Retrigger(false);
            adsrMod.Retrigger(false);
        }
        gate = true;
    }
}

void HandleNoteOff(uint8_t note_in)
{
    bool activeNoteChanged = false;

    // Go through all the active notes and remove any with this number
    for (int i = activeNoteCount - 1; i >= 0; i--)
    {
        if (activeNotes[i] == note_in)
        {
            // Found a match: is it the most recent note?
            if (i == activeNoteCount - 1)
            {
                activeNoteChanged = true;
            }

            for (int j = i; j < activeNoteCount - 1; j++)
            {
                activeNotes[j] = activeNotes[j + 1];
            }
            activeNoteCount--;
        }
    }
    if (activeNoteCount == 0)
    {
        // No notes left
        gate = false;
    }
    else if (activeNoteChanged)
    {
        // Update the frequency but don't retrigger
        int mostRecentNote = activeNotes[activeNoteCount - 1];
        frequency = midiNoteToFreqTable[mostRecentNote];
        noteNum = mostRecentNote;
        gate = true;
    }
}

void VoiceProcess(float &voice_sig)
{
    voice_sig = 0.0f;

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        pitch_correction[i] = GetPitchTableValue(paramManager.GetValue(OSC_PITCH[i]));
        detune_correction[i] = GetDetuneTableValue(paramManager.GetValue(OSC_DETUNE[i]));
        final_freq[i] = frequency * pitch_correction[i] * detune_correction[i] * pitch_bend_multiplier;
        paramManager.SetFloat(OSC_FREQ[i], final_freq[i]);

        osc[i].SetFreq(paramManager.GetValue(OSC_FREQ[i]));
        osc[i].SetAmp(paramManager.GetValue(OSC_AMP[i]));
        osc[i].SetWaveform(paramManager.GetValue(OSC_WAVEFORM[i]));
        osc[i].SetPw(paramManager.GetValue(OSC_PWM[i]));
        osc[i].PhaseProcess();

        if (paramManager.GetValue(OSC_ACTIVE[i]))
        {
            voice_sig += osc[i].Process();
        }
    }

    if (fabs(voice_sig) > 1.0f)
    {
        voice_sig = daisysp::fclamp(voice_sig, -1.0f, 1.0f);
    }

    flt.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    flt.SetRes(paramManager.GetValue(P::FILTER_RESONANCE));
    voice_sig = flt.Process(voice_sig);

    adsrMain.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
    adsrMain.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
    adsrMain.SetSustainLevel(paramManager.GetValue(P::ADSR_SUSTAIN));
    adsrMain.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));

    float env = adsrMain.Process(gate);
    voice_sig *= env;
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