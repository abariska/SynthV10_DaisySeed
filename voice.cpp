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

Adsr adsrMod;
LadderFilter fltL, fltR;
Osc lfo;
Random rnd[OSC_NUM * VOICE_NUM];
ModMatrix modMatrix[MOD_MATRIX_NUM];
Voice voice[VOICE_NUM];

uint8_t noteStack[MAX_NOTE_STACK];
uint8_t notesInStack = 0;

float midiNoteToFreqTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];
float freqModTable[FREQ_MOD_TABLE_SIZE];

float voice_pan[VOICE_NUM] = {
    0.0f, 0.5f, -0.5f, 1.0f, -1.0f};

float panningTable[PANNING_TABLE_SIZE][2] = {{0.0f}}; 

uint8_t noteNum = 60;
float frequency = 0;
float phaseOffsets[OSC_NUM * VOICE_NUM];
float prev_freq[OSC_NUM * VOICE_NUM] = {0.0f};
bool is_any_voice_active = false;

bool isOscSyncNeeded[OSC_NUM * VOICE_NUM] = {false};
bool gate = false;
float lfo_value = 0.0f;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    InitPanningTable();
    for (size_t i = 0; i < VOICE_NUM; i++)
    {
        for (size_t j = 0; j < OSC_NUM; j++)
        {
            voice[i].osc[j].Init(samplerate);
            voice[i].rnd[j].Init();
        }
        voice[i].adsr.Init(samplerate, blocksize);
        
    }
    fltL.Init(samplerate);
    fltR.Init(samplerate);
    adsrMod.Init(samplerate, blocksize);
    lfo.Init(samplerate, true);
    EffectsInit(samplerate);
}

void ModSourcesProcess()
{

    lfo_value = lfo.Process() * 0.5f + 0.5f;
    modulators[static_cast<int>(M::LFO)].value = lfo_value;
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

void PushNote(uint8_t note)
{
    for (int i = 0; i < notesInStack; i++)
    {
        if (noteStack[i] == note)
            return;
    }

    if (notesInStack < MAX_NOTE_STACK)
    {
        noteStack[notesInStack] = note;
        notesInStack++;
    }
    else
    {
        for (int i = 0; i < MAX_NOTE_STACK - 1; i++)
        {
            noteStack[i] = noteStack[i + 1];
        }
        noteStack[MAX_NOTE_STACK - 1] = note;
    }
}

uint8_t PopNote( uint8_t note)
{
    for (int i = 0; i < notesInStack; i++)
    {
        if (noteStack[i] == note)
        {
            for (int j = i; j < notesInStack - 1; j++)
            {
                noteStack[j] = noteStack[j + 1];
            }
            notesInStack--;
            break;
        }
    }
    return (notesInStack > 0) ? noteStack[notesInStack - 1] : 0;
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
        for(int v=0; v<VOICE_NUM; ++v)
            if(!voice[v].active) return v;

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
    if (!is_any_voice_active)
    {
        lfo.SyncPhaseToZero();
    }

    is_any_voice_active = true;
    gate = true;

    if (paramManager.GetBool(P::GLOBAL_MONO))
    {
        PushNote(note_in);
    }
    
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
}

void HandleNoteOff(uint8_t note_in)
{
    if (paramManager.GetBool(P::GLOBAL_MONO))
    {
        if (voice[0].note != note_in)
        {
            PopNote(note_in);
            return;
        }

        uint8_t prevNote = PopNote(note_in);
        
        if (prevNote > 0)
        {
            voice[0].note = prevNote;
            voice[0].freq = midiNoteToFreqTable[prevNote];
            voice[0].gate = true;
            voice[0].timestamp = System::GetNow();
            
            if (!paramManager.GetBool(P::GLOBAL_LEGATO))
            {
                voice[0].adsr.Retrigger(false);
            }
            gate = true;
            return;
        }
    }

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
    int active_voices = 0;
    for (int v = 0; v < VOICE_NUM; ++v)
    {
        if (voice[v].active && voice[v].gate)
        {
            active_voices++;
        }
    }
    if (active_voices > 0)
    {
        is_any_voice_active = true;
    } else {
        is_any_voice_active = false;
    }
    gate = is_any_voice_active;
}

void UpdateSynthParams()
{
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        // if the voice is not active, skip it
        if (!voice[v].active) continue;

        const float voiceFreq = voice[v].freq;
        const float voiceVel = voice[v].vel;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            phaseOffsets[v * OSC_NUM + oscId] = rnd[v].GetFloat(0.0f, 0.000001f);
            float pitch = GetPitchTableValue(paramManager.GetValue(OSC_PITCH[oscId]));
            float detune = GetDetuneTableValue(paramManager.GetValue(OSC_DETUNE[oscId]));
            voice[v].final_freq[oscId] = voiceFreq * pitch * detune * pitch_bend_multiplier;
            paramManager.SetValue(OSC_FREQ[oscId], voice[v].final_freq[oscId]);
            float amp = paramManager.GetValue(OSC_AMP[oscId]) * voiceVel;
            float pw = paramManager.GetValue(OSC_PWM[oscId]);
            int waveform = static_cast<int>(paramManager.GetValue(OSC_WAVEFORM[oscId]));

            voice[v].osc[oscId].SetActive(paramManager.GetValue(OSC_ACTIVE[oscId]));
            voice[v].osc[oscId].SetFreq(paramManager.GetValue(OSC_FREQ[oscId]));    
            voice[v].osc[oscId].SetAmp(amp);
            voice[v].osc[oscId].SetWaveform(waveform);
            voice[v].osc[oscId].SetPw(pw);
            voice[v].osc[oscId].SetPortamento(paramManager.GetValue(P::GLOBAL_PORTAMENTO));
            if (voice[v].final_freq[oscId] != prev_freq[v * OSC_NUM + oscId])
            {
                isOscSyncNeeded[v * OSC_NUM + oscId] = true;
                prev_freq[v * OSC_NUM + oscId] = voice[v].final_freq[oscId];
            }
        }
        voice[v].adsr.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
        voice[v].adsr.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
        voice[v].adsr.SetSustainLevel(paramManager.GetValue(P::ADSR_SUSTAIN));
        voice[v].adsr.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));
    }

    fltL.SetFilterMode(static_cast<LadderFilter::FilterMode>(paramManager.GetValue(P::FILTER_MODE)));
    fltL.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    fltL.SetRes(paramManager.GetValue(P::FILTER_RESONANCE));
    fltL.SetPassbandGain(0.5f);
    fltL.SetInputDrive(1.0f + (paramManager.GetValue(P::FILTER_DRIVE) * 4.0f));
    fltR.SetFilterMode(static_cast<LadderFilter::FilterMode>(paramManager.GetValue(P::FILTER_MODE)));
    fltR.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    fltR.SetRes(paramManager.GetValue(P::FILTER_RESONANCE));
    fltR.SetPassbandGain(0.5f);
    fltR.SetInputDrive(1.0f + (paramManager.GetValue(P::FILTER_DRIVE) * 4.0f));

    fx.flanger.SetFeedback(paramManager.GetValue(P::EFFECT_FLANGER_FEEDBACK));
    fx.flanger.SetLfoDepth(paramManager.GetValue(P::EFFECT_FLANGER_LFO_DEPTH));
    fx.flanger.SetLfoFreq(paramManager.GetValue(P::EFFECT_FLANGER_LFO_FREQ));
    fx.flanger.SetDelay(paramManager.GetValue(P::EFFECT_FLANGER_DELAY));

    fx.wah.SetWah(paramManager.GetValue(P::EFFECT_AUTOWAH_WAH));
    fx.wah.SetLevel(paramManager.GetValue(P::EFFECT_AUTOWAH_LEVEL));

    fx.drive.SetDrive(paramManager.GetValue(P::EFFECT_OVERDRIVE_DRIVE));

    fx.chorus.SetLfoFreq(paramManager.GetValue(P::EFFECT_CHORUS_FREQ));
    fx.chorus.SetLfoDepth(paramManager.GetValue(P::EFFECT_CHORUS_DEPTH));
    fx.chorus.SetFeedback(paramManager.GetValue(P::EFFECT_CHORUS_FBK));
    fx.chorus.SetDelay(paramManager.GetValue(P::EFFECT_CHORUS_DELAY));

    fx.compressor.SetAttack(paramManager.GetValue(P::EFFECT_COMPRESSOR_ATTACK));
    fx.compressor.SetRelease(paramManager.GetValue(P::EFFECT_COMPRESSOR_RELEASE));
    fx.compressor.SetThreshold(paramManager.GetValue(P::EFFECT_COMPRESSOR_THRESHOLD));
    fx.compressor.SetRatio(paramManager.GetValue(P::EFFECT_COMPRESSOR_RATIO));
    fx.compressor.SetMakeup(paramManager.GetValue(P::EFFECT_COMPRESSOR_MAKEUP));

    fx.reverb.SetFeedback(paramManager.GetValue(P::EFFECT_REVERB_FEEDBACK));
    fx.reverb.SetLpFreq(paramManager.GetValue(P::EFFECT_REVERB_LPFREQ));
}

void VoiceProcess(float &out_sigL, float &out_sigR)
{
    float outL = 0.0f;
    float outR = 0.0f;
    
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        float voice_out = 0.0f;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            if (isOscSyncNeeded[v * OSC_NUM + oscId])
            {
                float phase = voice[v].osc[0].GetPhase();
                if (oscId != 0)
                { 
                    voice[v].osc[oscId].SyncPhase(phase);
                }

                isOscSyncNeeded[v * OSC_NUM + oscId] = false; 
            }

            float process_out = voice[v].osc[oscId].Process();
            voice_out += process_out;
        }
        
        float env = voice[v].adsr.Process(voice[v].gate);
        float voice_out_env = voice_out * env / VOICE_NUM;
        float voiceL, voiceR;
        VoicePanning(v, voice_out_env, voiceL, voiceR);
        outL += voiceL;
        outR += voiceR;
        
        // if the voice is not active and the envelope is below 0.00001f, kill the voice
        if (!(voice[v].active && voice[v].gate) && env <= 0.00001f) 
        {
            voice[v].gate = false;
            voice[v].active = false;
        }
    }
    
    outL = fltL.Process(outL);   
    outR = fltR.Process(outR);

    outL = softClip(outL);
    outR = softClip(outR);

    out_sigL = outL;
    out_sigR = outR;
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

inline float softClip(float x)
{
    if (x > 1.0f)  return 1.0f - 1.0f / (x + 1.0f);
    if (x < -1.0f) return -1.0f - 1.0f / (x - 1.0f);
    return x;
}

inline void InitPanningTable()
{
    for (int i = 0; i <= PANNING_TABLE_SIZE; i++) {
        float t = (float)i / PANNING_TABLE_SIZE;
        panningTable[i][0] = sqrtf(t);
        panningTable[i][1] = sqrtf(1.0f - t);
    }
}

inline void VoicePanning(uint8_t voice_num, float &voice_sig, float &out_L, float &out_R)
{
    float globalPan = paramManager.GetValue(P::GLOBAL_PAN);
    
    // Обчислити фінальну позицію: voice_pan * globalPan
    float panPos = voice_pan[voice_num] * globalPan;  // -1..1
    
    // Конвертувати -1..1 → 0..1 для sqrt
    float panNorm = (panPos + 1.0f) * 0.5f;  // 0..1
    
    int idx = (int)(panNorm * PANNING_TABLE_SIZE); 
    
    // З таблиці
    out_L = voice_sig * panningTable[idx][0];
    out_R = voice_sig * panningTable[idx][1];
}
