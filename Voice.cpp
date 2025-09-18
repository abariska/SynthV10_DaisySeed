#include "voice.h"
#include "main.h"
#include "daisy_seed.h"
#include "oscillator.h"

std::array<Osc, OSC_NUM> osc;
Adsr adsrMain;
MoogLadder fltL;
MoogLadder fltR;
Oscillator lfo;
Random rnd[OSC_NUM];

using P = ParamUnitName;

uint8_t noteNum = 60;
float phase = 0;
float frequency = 0;
float masterPhase = 0.0f;
float oscPhase[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float oscPhaseInc[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float phaseOffsets[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float pitch_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float detune_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float final_freq[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float amplitude[OSC_NUM] = {0.0f, 0.0f, 0.0f};

const int maxNotes = 16;
int activeNotes[maxNotes];
int activeNoteCount = 0;
bool gate = false;
bool hardRetrigger = false;

float ProcessLfo()
{
    // Apply parameters from template
    lfo.SetFreq(paramManager.GetValue(P::LFO_FREQ));
    lfo.SetWaveform(paramManager.GetInt(P::LFO_WAVEFORM));
    lfo.SetAmp(paramManager.GetNormalised(P::LFO_DEPTH));
    // if (paramManager.GetInt(P::LFO_ACTIVE) == 0) return 0.0f;
    return lfo.Process();
}

void SynthInit(float samplerate, int blocksize)
{

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        osc[i].Init(samplerate);
    }
    fltL.Init(samplerate);
    fltR.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);
    for (size_t i = 0; i < OSC_NUM; i++)
    {
        rnd[i].Init();
    }
    lfo.Init(samplerate);
}

void HandleNoteOn(uint8_t note_in, uint8_t velocity)
{
    bool isNotesPlaying = (activeNoteCount > 0);
    // Add this note to the list of active notes
    if (activeNoteCount < maxNotes)
    {
        activeNotes[activeNoteCount] = note_in;
        activeNoteCount++;

        noteNum = note_in;
        frequency = 440.0f * powf(2.0f, (note_in - 69) / 12.0f);
        float velocity_factor = velocity / 127.0f;

        for (size_t i = 0; i < OSC_NUM; i++)
        {
            phaseOffsets[i] = rnd[i].GetFloat(0.0f, 0.0000001f);
            pitch_correction[i] = powf(2.0f, paramManager.GetInt(OSC_PITCH[i]) / 12.0f);
            detune_correction[i] = powf(2.0f, paramManager.GetInt(OSC_DETUNE[i]) / 1200.0f);
            final_freq[i] = frequency * pitch_correction[i] * detune_correction[i];

            osc[i].SetPhaseOffset(phaseOffsets[i]);
            osc[i].SyncToMaster(masterPhase);

            float freq_compensation = 1.0f;
            amplitude[i] = velocity_factor * freq_compensation;
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

void VoiceProcess(float &voice_sig)
{
    voice_sig = 0.0f;
    int active_osc_count = 0;
    float masterInc = frequency / samplerate; // тут frequency = основна нота
    masterPhase += masterInc;
    if (masterPhase >= 1.0f)
    {
        masterPhase -= 1.0f;
    }

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        osc[i].SetFreq(final_freq[i]);
        osc[i].SetAmp(paramManager.GetNormalised(OSC_AMP[i]) * amplitude[i]);
        osc[i].SetWaveform(paramManager.GetInt(OSC_WAVEFORM[i]));
        osc[i].SetPw(paramManager.GetNormalised(OSC_PWM[i]));

        if (paramManager.GetValue(OSC_ACTIVE[i]))
        {
            voice_sig += osc[i].Process();
            active_osc_count++;
        }
    }

    if (active_osc_count > 0)
    {
        voice_sig /= active_osc_count;
    }
    else
    {
        voice_sig = 0.0f; // Явне вимкнення
    }

    if (fabs(voice_sig) > 1.0f)
    {
        voice_sig = daisysp::fclamp(voice_sig, -1.0f, 1.0f);
    }

    // float lfoSig = ProcessLfo();

    fltR.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    fltR.SetRes(paramManager.GetNormalised(P::FILTER_RESONANCE));
    voice_sig = fltR.Process(voice_sig);

    // TODO: fix the real curve. it sounds shorter than it should be.
    adsrMain.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
    adsrMain.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
    adsrMain.SetSustainLevel(paramManager.GetNormalised(P::ADSR_SUSTAIN));
    adsrMain.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));

    float env = adsrMain.Process(gate);
    voice_sig *= env;
}
