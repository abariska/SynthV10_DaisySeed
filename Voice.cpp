#include "voice.h"
#include "main.h"
#include "daisy_seed.h"
#include "oscillator.h"
#include "pitchTables.h"
#include "parameters.h"

using P = ParamUnitName;

using M = ModSource;

std::array<Osc, OSC_NUM> osc;
Adsr adsrMain;
Adsr adsrMod;
MoogLadder flt;
Oscillator lfo;
Random rnd[OSC_NUM];
ModMatrix modMatrix[MOD_MATRIX_NUM];

uint8_t noteNum = 60;
float frequency = 0;
float phaseOffsets[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float pitch_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float detune_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float final_freq[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float velocity_factor = 1.0f;
float pitch_bend_multiplier = 1.0f;

const int maxNotes = 16;
int activeNotes[maxNotes];
int activeNoteCount = 0;
bool gate = false;

void SynthInit(float samplerate, int blocksize)
{

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
    InitPitchTables();
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

    modulators[static_cast<int>(M::MOD_WHEEL)].value = 0.0f;
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

        noteNum = mostRecentNote;
        gate = true;
    }
}

void HandlePitchBend(int16_t pb)
{
    float bend_cents = ((float)(pb - 8192) / 8192.0f) * 200.0f;
    pitch_bend_multiplier = GetPitchBendTableValue(bend_cents);
}

void VoiceProcess(float &voice_sig)
{
    voice_sig = 0.0f;

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        pitch_correction[i] = GetPitchTableValue(paramManager.GetValue(OSC_PITCH[i]));
        detune_correction[i] = GetDetuneTableValue(paramManager.GetValue(OSC_DETUNE[i]));
        final_freq[i] = frequency * pitch_correction[i] * detune_correction[i] * pitch_bend_multiplier;

        osc[i].SetFreq(final_freq[i]);
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