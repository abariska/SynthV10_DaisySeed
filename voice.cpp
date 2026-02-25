#include "voice.h"
#include "midi_handler.h"


#define DTCM __attribute__((section(".dtcmram_bss")))

using P = ParamUnitName;
using namespace daisy;
using M = ModSource;

Adsr adsrMod;
LadderFilter flt[2];
OscLfo lfo;
Random rnd;
ModMatrix modMatrix[MOD_MATRIX_NUM];
Voice voice[VOICE_NUM];

uint8_t noteStack[MAX_NOTE_STACK];
uint8_t notesInStack = 0;

float midiNoteToFreqTable[128];
float velocityToAmpTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];
float freqModTable[FREQ_MOD_TABLE_SIZE];

static float cached_pitch[OSC_NUM];
static float cached_detune[OSC_NUM];
static float osc_freq_factor[OSC_NUM] = {1.0f};
static float cached_portamento = 0.0f;
static bool cached_mono = false;
static bool cached_legato = false;
static float cached_pan = 0.0f;
static bool cached_lfo_trigger = false;
float cached_master_volume = 0.0f;

static int cached_waveform[OSC_NUM];
static bool cached_active[OSC_NUM];
static float cached_pw[OSC_NUM];
static float cached_amp[OSC_NUM];
static float cached_pan_correction[VOICE_NUM][2]; // 0 - left, 1 - right

float panningTable[PANNING_TABLE_SIZE][2] = {{0.0f}}; 
float voice_pan_range[VOICE_NUM] = {
    0.0f, 0.5f, -0.5f, 1.0f, -1.0f};

uint8_t noteNum = 60;
float frequency = 0;
bool is_any_voice_active = false;
bool polyToMonoSwitch = false;

bool isOscSyncNeeded[OSC_NUM] = {true};
bool gate = false;
float lfo_value = 0.0f;
bool isVoiceActive[VOICE_NUM] = {false};
int isModAffectsOscFreq = 0;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    InitPanningTable();
    rnd.Init();
    for (size_t i = 0; i < VOICE_NUM; i++)
    {
        voice[i].phaseOffset = rnd.GetFloat(0.0f, 0.5f);
        for (size_t j = 0; j < OSC_NUM; j++)
        {
            voice[i].osc[j].Init(samplerate);
            voice[i].rnd[j].Init();
            voice[i].osc[j].SetPhaseOffset(voice[i].phaseOffset);
        }
        voice[i].adsr.Init(samplerate, blocksize);
        
    }
    flt[0].Init(samplerate);
    flt[1].Init(samplerate);
    adsrMod.Init(samplerate, blocksize);
    lfo.Init(samplerate);
    EffectsInit(samplerate);
    VoicePanningInit();
}

void ModSourcesProcess()
{
    lfo_value = lfo.Process();
    modulators[static_cast<int>(M::LFO)].value = lfo_value;
    modulators[static_cast<int>(M::ADSR)].value = adsrMod.Process(gate);

    modulators[static_cast<int>(M::MOD_WHEEL)].value = mod_wheel_value;
    modulators[static_cast<int>(M::AFTERTOUCH)].value = aftertouch_value;
}

void UpdateModSourcesParams()
{
    if (dirty.modLfoParams) 
    {
        lfo.SetFreq(paramManager.GetValue(P::MOD_LFO_FREQ));
        lfo.SetWaveform(paramManager.GetValue(P::MOD_LFO_WAVEFORM));
        lfo.SetAmp(paramManager.GetValue(P::MOD_LFO_DEPTH));
        cached_lfo_trigger = paramManager.GetBool(P::MOD_LFO_TRIGGER);
        dirty.modLfoParams = false;
    }
    else if (dirty.modAdsrParams) {
        adsrMod.SetAttackTime(paramManager.GetValue(P::MOD_ADSR_ATTACK), 1.0f);
        adsrMod.SetDecayTime(paramManager.GetValue(P::MOD_ADSR_DECAY));
        adsrMod.SetSustainLevel(paramManager.GetValue(P::MOD_ADSR_SUSTAIN));
        adsrMod.SetReleaseTime(paramManager.GetValue(P::MOD_ADSR_RELEASE));
        dirty.modAdsrParams = false;
    }
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
    if (!cached_mono)
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
    if (cached_lfo_trigger)
    {
        if (!is_any_voice_active) lfo.SyncPhaseToStart();
    }

    is_any_voice_active = true;
    gate = true;

    if (cached_mono)
    {
        PushNote(note_in);
    }
    
    int v = AllocVoice();
    voice[v].active = true;
    voice[v].note = note_in;
    voice[v].freq = midiNoteToFreqTable[note_in];
    voice[v].vel = velocityToAmpTable[velocity];
    voice[v].gate = true;
    voice[v].timestamp = System::GetNow();
    
    if (!cached_legato)
    {
        voice[v].adsr.Retrigger(false);
        adsrMod.Retrigger(false);
    }
    dirty.oscParams = true;
    dirty.adsrParams = true;
    isVoiceActive[v] = true;
    isModAffectsOscFreq++;
}


// TODO: click on NoteOff when Mono and chord is playing  
void HandleNoteOff(uint8_t note_in)
{
    if (cached_mono)
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
            
            if (!cached_legato)
            {
                voice[0].adsr.Retrigger(false);
            }
            dirty.adsrParams = true; 
            dirty.oscParams = true;
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
            isVoiceActive[v] = false;
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
    if (dirty.globalParams)
    {
        if (paramManager.GetParam(P::GLOBAL_PORTAMENTO).isDirty)
        {
            cached_portamento = paramManager.GetValue(P::GLOBAL_PORTAMENTO);
            paramManager.GetParam(P::GLOBAL_PORTAMENTO).isDirty = false;
            dirty.oscParams = true;
        } 
        
        if (paramManager.GetParam(P::GLOBAL_MONO).isDirty)
        {
            bool new_mono = paramManager.GetBool(P::GLOBAL_MONO);
            if (new_mono != cached_mono)
            {
                cached_mono = new_mono;
                polyToMonoSwitch = new_mono;
            }
            paramManager.GetParam(P::GLOBAL_MONO).isDirty = false;
            dirty.oscParams = true;
        }

        if (paramManager.GetParam(P::GLOBAL_PAN).isDirty)
        {
            cached_pan = paramManager.GetValue(P::GLOBAL_PAN);
            paramManager.GetParam(P::GLOBAL_PAN).isDirty = false;
            VoicePanningInit();
        }
        
        cached_legato = paramManager.GetBool(P::GLOBAL_LEGATO);
        cached_master_volume = paramManager.GetValue(P::GLOBAL_MASTER_VOLUME);
        dirty.globalParams = false;
    }

    if (dirty.oscParams)
    {
        if (polyToMonoSwitch)
        {
            for (size_t v = 0; v < VOICE_NUM; ++v)
            {
                SynthVoiceReset(v);
            }
            notesInStack = 0;
            is_any_voice_active = false;
            gate = false;
            polyToMonoSwitch = false;
        } 
        else {
            for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
            {
                static float osc_data_prev[OSC_NUM] = {0.0f};
                float osc_pitch = paramManager.GetValue(OSC_PITCH[oscId]);
                float osc_detune = paramManager.GetValue(OSC_DETUNE[oscId]);
                float osc_data = osc_pitch + osc_detune;

                if (fabsf(osc_data - osc_data_prev[oscId]) > 0.000001f)
                {
                    isOscSyncNeeded[oscId] = true;
                    if (oscId == 0) 
                    {
                        isOscSyncNeeded[1] = true;
                        isOscSyncNeeded[2] = true;
                    }
                } 
                osc_data_prev[oscId] = osc_data;

                cached_pitch[oscId] = GetPitchTableValue(osc_pitch);
                cached_detune[oscId] = GetDetuneTableValue(osc_detune);   
                osc_freq_factor[oscId] = cached_pitch[oscId] * cached_detune[oscId];

                cached_waveform[oscId] = (paramManager.GetValue(OSC_WAVEFORM[oscId]));
                cached_pw[oscId] = paramManager.GetValue(OSC_PWM[oscId]);
                cached_active[oscId] = paramManager.GetValue(OSC_ACTIVE[oscId]);
                cached_amp[oscId] = paramManager.GetValue(OSC_AMP[oscId]);
            }

            for (size_t v = 0; v < VOICE_NUM; ++v)
            {
                for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
                {
                    // phaseOffsets[v * OSC_NUM + oscId] = rnd[v].GetFloat(0.0f, 0.000001f);
                    voice[v].osc[oscId].SetActive(cached_active[oscId]);
                    voice[v].osc[oscId].SetWaveform(cached_waveform[oscId]);
                    voice[v].osc[oscId].SetPw(cached_pw[oscId]);
                    voice[v].osc[oscId].SetPortamento(cached_portamento);
                }   
                if (dirty.adsrParams)
                {
                    voice[v].adsr.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
                    voice[v].adsr.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
                    voice[v].adsr.SetSustainLevel(paramManager.GetValue(P::ADSR_SUSTAIN));
                    voice[v].adsr.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));
                }
                
            }
            polyToMonoSwitch = false;
        }
    }
    dirty.oscParams = false;
    dirty.adsrParams = false;
    
    if (dirty.filterParams)
    {
        LadderFilter::FilterMode mode = static_cast<LadderFilter::FilterMode>(paramManager.GetValue(P::FILTER_MODE));
        float filter_cutoff = paramManager.GetValue(P::FILTER_CUTOFF);
        float filter_resonance = paramManager.GetValue(P::FILTER_RESONANCE);
        float filter_drive = paramManager.GetValue(P::FILTER_DRIVE);
        float input_drive = 1.0f + (filter_drive * 4.0f);

        for (int i = 0; i < 2; i++)
        {
            flt[i].SetFilterMode(mode);
            flt[i].SetFreq(filter_cutoff);
            flt[i].SetRes(filter_resonance);
            flt[i].SetPassbandGain(0.5f);
            flt[i].SetInputDrive(input_drive);
        }
        dirty.filterParams = false;
    }
    if (dirty.flangerParams)
    {
        fx.flanger.SetFeedback(paramManager.GetValue(P::EFFECT_FLANGER_FEEDBACK));
        fx.flanger.SetLfoDepth(paramManager.GetValue(P::EFFECT_FLANGER_LFO_DEPTH));
        fx.flanger.SetLfoFreq(paramManager.GetValue(P::EFFECT_FLANGER_LFO_FREQ));
        fx.flanger.SetDelay(paramManager.GetValue(P::EFFECT_FLANGER_DELAY));
        dirty.flangerParams = false;
    }
    if (dirty.wahParams)
    {
        fx.wah.SetWah(paramManager.GetValue(P::EFFECT_AUTOWAH_WAH));
        fx.wah.SetLevel(paramManager.GetValue(P::EFFECT_AUTOWAH_LEVEL));
        dirty.wahParams = false;
    }
    if (dirty.driveParams)
    {
        fx.drive.SetDrive(paramManager.GetValue(P::EFFECT_OVERDRIVE_DRIVE));
        dirty.driveParams = false;
    }
    if (dirty.chorusParams)
    {
        fx.chorus.SetLfoFreq(paramManager.GetValue(P::EFFECT_CHORUS_FREQ));
        fx.chorus.SetLfoDepth(paramManager.GetValue(P::EFFECT_CHORUS_DEPTH));
        fx.chorus.SetFeedback(paramManager.GetValue(P::EFFECT_CHORUS_FBK));
        fx.chorus.SetDelay(paramManager.GetValue(P::EFFECT_CHORUS_DELAY));
        dirty.chorusParams = false;
    }
    if (dirty.compressorParams)
    {
        fx.compressor.SetAttack(paramManager.GetValue(P::EFFECT_COMPRESSOR_ATTACK));
        fx.compressor.SetRelease(paramManager.GetValue(P::EFFECT_COMPRESSOR_RELEASE));
        fx.compressor.SetThreshold(paramManager.GetValue(P::EFFECT_COMPRESSOR_THRESHOLD));
        fx.compressor.SetRatio(paramManager.GetValue(P::EFFECT_COMPRESSOR_RATIO));
        fx.compressor.SetMakeup(paramManager.GetValue(P::EFFECT_COMPRESSOR_MAKEUP));
        dirty.compressorParams = false;
    }
    if (dirty.reverbParams)
    {
        fx.reverb.SetFeedback(paramManager.GetValue(P::EFFECT_REVERB_FEEDBACK));
        fx.reverb.SetLpFreq(paramManager.GetValue(P::EFFECT_REVERB_LPFREQ));
        dirty.reverbParams = false;
    }
}

void VoiceProcess(float &out_sigL, float &out_sigR)
{
    float outL = 0.0f;
    float outR = 0.0f;

    float modifier[OSC_NUM] = {0.0f};
    float freq_factor[OSC_NUM] = {0.0f};
    for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
    {
        modifier[oscId] = paramManager.GetModifier(OSC_FREQ[oscId]);
        freq_factor[oscId] = osc_freq_factor[oscId] * pitch_bend_multiplier * modifier[oscId];
    }

    for (size_t v = 0; v < VOICE_NUM; ++v)
    {   
        float phase = voice[v].osc[0].GetPhase();
        float voice_out = 0.0f;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            float freq = voice[v].freq * freq_factor[oscId];
            voice[v].osc[oscId].SetFreq(freq);
            voice[v].osc[oscId].SetAmp(cached_amp[oscId] * voice[v].vel);
            if (isOscSyncNeeded[oscId] || isModAffectsOscFreq > 0)
            {
                if (oscId != 0)
                { 
                    voice[v].osc[oscId].SyncPhase(phase);
                }
                if (v == VOICE_NUM - 1) 
                {
                    isOscSyncNeeded[oscId] = false; 
                }
            }

            float osc_out = voice[v].osc[oscId].Process();
            voice_out += osc_out;
        }
        float env = voice[v].adsr.Process(voice[v].gate);
        float voice_out_env = voice_out * env / VOICE_NUM;
        outL += voice_out_env * cached_pan_correction[v][0];
        outR += voice_out_env * cached_pan_correction[v][1];
        
        // if the voice is not active and the envelope is below 0.00001f, kill the voice
        if (!(voice[v].active && voice[v].gate) && env <= 0.000001f) 
        {
            voice[v].gate = false;
            voice[v].active = false;
        }
    }
    isModAffectsOscFreq = 0;
    
    outL = flt[0].Process(outL);   
    outR = flt[1].Process(outR);

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
    for (int i = 0; i < 128; i++)
    {
        velocityToAmpTable[i] = i / 127.0f;
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

float GetVelocityToAmpTableValue(uint8_t velocity)
{
    return velocityToAmpTable[velocity];
}

inline float softClip(float x)
{
    if (x > 1.0f)  return 1.0f - 1.0f / (x + 1.0f);
    if (x < -1.0f) return -1.0f - 1.0f / (x - 1.0f);
    return x;
}

inline void InitPanningTable()
{
    for (int i = 0; i < PANNING_TABLE_SIZE; i++) {
        float t = (float)i / (PANNING_TABLE_SIZE - 1);
        panningTable[i][0] = sqrtf(1.0f - t);
        panningTable[i][1] = sqrtf(t);
    }
}

inline void VoicePanningInit()
{
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        // Обчислити фінальну позицію: voice_pan * globalPan
        float panPos = voice_pan_range[v] * cached_pan;  // -1..1
        // Конвертувати -1..1 → 0..1 для sqrt
        float panNorm = (panPos + 1.0f) * 0.5f;  // 0..1
        int idx = (int)(panNorm * (PANNING_TABLE_SIZE - 1)); 
        cached_pan_correction[v][0] = panningTable[idx][0];
        cached_pan_correction[v][1] = panningTable[idx][1];
    }
}

void SynthVoiceReset(uint8_t voice_num){
    voice[voice_num].active = false;
    voice[voice_num].gate = false;
    voice[voice_num].note = 0;
    voice[voice_num].freq = 0.0f;
    voice[voice_num].vel = 0.0f;
    voice[voice_num].timestamp = 0;
    isVoiceActive[voice_num] = false;
}

void ModMatrixReset(uint8_t mod_matrix_num){
    modMatrix[mod_matrix_num].ResetMods();
}