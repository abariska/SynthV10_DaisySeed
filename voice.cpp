#include "voice.h"
#include "midi_handler.h"

using P = ParamUnitName;
using namespace daisy;
using M = ModSource;

Adsr adsrModGlobal;
OscLfo lfo;
Random rnd;
Voice voice[VOICE_NUM];

uint8_t noteStack[MAX_NOTE_STACK];
uint8_t notesInStack;

float midiNoteToFreqTable[128];
float velocityToAmpTable[128];
float pitchTable[PITCH_TABLE_SIZE];
float detuneTable[DETUNE_TABLE_SIZE];
float pitchBendTable[PITCH_BEND_TABLE_SIZE];
float freqModTable[FREQ_MOD_TABLE_SIZE];

static bool cached_lfo_trigger = false;

GlobalCache cache_global;
OscCache cache_osc[OSC_NUM];
bool is_osc_dirty = true;
bool is_flt_dirty = true;
bool is_env_dirty = true;
bool is_voice_dirty = true;
VoiceCache cache_voice[VOICE_NUM];
ModCache cache_mod;

float panningTable[PANNING_TABLE_SIZE][2] = {{0.0f}}; 
float voice_pan_range[VOICE_NUM] = {
    0.0f, 0.5f, -0.5f, 1.0f, -1.0f};

static const float inv_voice_num = 1.0f / VOICE_NUM;

uint8_t noteNum = 60;
float frequency;
bool is_any_voice_active = false;
bool polyToMonoSwitch = false;
bool isOscSyncNeeded[OSC_NUM] = {true};
bool gate = false;
float lfo_value;
int isModAffectsOscFreq = 0;

void SynthInit(float samplerate, int blocksize)
{
    InitPitchTables();
    InitPanningTable();
    rnd.Init(); 
    lfo.Init(samplerate);
    adsrModGlobal.Init(samplerate, blocksize);

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
        voice[i].adsrMod.Init(samplerate, blocksize);
        voice[i].flt.Init(samplerate);
    }
    
    VoicePanningInit();
    ResetModModulators();
    EffectsInit(samplerate);
}

void ModSourcesProcess()
{
    lfo_value = lfo.Process();
    modulators[static_cast<int>(M::LFO)].value = lfo_value;

    float adsr_value = adsrModGlobal.Process(gate);
    modulators[static_cast<int>(M::ADSR)].value = adsr_value;

    float wheel_value = mod_wheel_value;
    modulators[static_cast<int>(M::MOD_WHEEL)].value = wheel_value;

    float at_value = aftertouch_value;
    modulators[static_cast<int>(M::AFTERTOUCH)].value = at_value;

    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        float adsr_value_per_voice = voice[v].adsrMod.Process(voice[v].gate);
        modulators[static_cast<int>(M::ADSR)].value_per_voice[v] = adsr_value_per_voice;
        
        float velocity_value_per_voice = voice[v].vel;
        modulators[static_cast<int>(M::VELOCITY)].value_per_voice[v] = velocity_value_per_voice;
    }

    for (size_t i = 0; i < MOD_MATRIX_NUM; ++i)
    {
        ParamUnitName targetParam = currentPreset.modMtx[i].modTarget;
        int sourceIndex = static_cast<int>(currentPreset.modMtx[i].modSource); 
        float modAmount = currentPreset.modMtx[i].modAmount;

        if (fabsf(modAmount) < 1e-5f) 
        {
            currentPreset.modMtx[i].modAmount = 0.0f;
            continue;
        }
        if (paramManager.GetIsPerVoice(targetParam))
        {
            for (size_t v = 0; v < VOICE_NUM; ++v)
            {
                float value = 0.0f;
                if (modulators[sourceIndex].is_per_voice)
                {
                    value = modulators[sourceIndex].value_per_voice[v] * modAmount;
                }
                else
                {
                    value = modulators[sourceIndex].value * modAmount;
                }
                paramManager.SetModifierPerVoice(targetParam, v, value);
            }
        }
        else
        {
            float value = modulators[sourceIndex].value * modAmount;
            paramManager.SetModifier(targetParam, value);
        }
        SetAudioDirtyFlag(targetParam);
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
    if (!cache_global.mono)
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

    if (cache_global.mono)
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
    
    if (!cache_global.legato)
    {
        voice[v].adsr.Retrigger(false);
        voice[v].adsrMod.Retrigger(false);
        adsrModGlobal.Retrigger(false);
    }
    dirty.oscParams = true;
    is_voice_dirty = true;
    isModAffectsOscFreq++;
}

// TODO: click on NoteOff when Mono and chord is playing  
void HandleNoteOff(uint8_t note_in)
{
    if (cache_global.mono)
    {
        if (voice[0].note != note_in)
        {
            PopNote(note_in);
            dirty.oscParams = true;
            return;
        }

        uint8_t prevNote = PopNote(note_in);
        
        if (prevNote > 0)
        {
            voice[0].note = prevNote;
            voice[0].freq = midiNoteToFreqTable[prevNote];
            voice[0].gate = true;
            voice[0].timestamp = System::GetNow();
            
            if (!cache_global.legato)
            {
                voice[0].adsr.Retrigger(false);
                voice[0].adsrMod.Retrigger(false);
                adsrModGlobal.Retrigger(false);
            }
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
    dirty.oscParams = true;
    gate = is_any_voice_active;
    is_voice_dirty = true;
}

void UpdateSynthParams()
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
        adsrModGlobal.SetAttackTime(paramManager.GetValue(P::MOD_ADSR_ATTACK), 1.0f);
        adsrModGlobal.SetDecayTime(paramManager.GetValue(P::MOD_ADSR_DECAY));
        adsrModGlobal.SetSustainLevel(paramManager.GetValue(P::MOD_ADSR_SUSTAIN));
        adsrModGlobal.SetReleaseTime(paramManager.GetValue(P::MOD_ADSR_RELEASE));
        for (size_t v = 0; v < VOICE_NUM; ++v)
        {
            voice[v].adsrMod.SetAttackTime(paramManager.GetValue(P::MOD_ADSR_ATTACK), 1.0f);
            voice[v].adsrMod.SetDecayTime(paramManager.GetValue(P::MOD_ADSR_DECAY));
            voice[v].adsrMod.SetSustainLevel(paramManager.GetValue(P::MOD_ADSR_SUSTAIN));
            voice[v].adsrMod.SetReleaseTime(paramManager.GetValue(P::MOD_ADSR_RELEASE));
        }
        dirty.modAdsrParams = false;
    }

    if (dirty.globalParams)
    {
        if (paramManager.GetDirty(P::GLOBAL_PORTAMENTO))
        {
            cache_global.portamento = paramManager.GetValue(P::GLOBAL_PORTAMENTO);
            paramManager.SetDirty(P::GLOBAL_PORTAMENTO, false);
            dirty.oscParams = true;
        } 
        
        if (paramManager.GetDirty(P::GLOBAL_MONO))
        {
            bool new_mono = paramManager.GetBool(P::GLOBAL_MONO);
            if (new_mono != cache_global.mono)
            {
                cache_global.mono = new_mono;
                polyToMonoSwitch = new_mono;
            }
            paramManager.SetDirty(P::GLOBAL_MONO, false);
            dirty.oscParams = true;
        }

        if (paramManager.GetDirty(P::GLOBAL_PAN))
        {
            cache_global.pan = paramManager.GetValue(P::GLOBAL_PAN);
            paramManager.SetDirty(P::GLOBAL_PAN, false);
            VoicePanningInit();
        }

        cache_global.legato = paramManager.GetBool(P::GLOBAL_LEGATO);
        cache_global.master_volume = paramManager.GetValue(P::GLOBAL_MASTER_VOLUME);
        dirty.globalParams = false;
    }

    dirty.oscParams = true;
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
                float osc_amp = paramManager.GetValue(OSC_AMP[oscId]);
                float osc_pw = paramManager.GetValue(OSC_PWM[oscId]);
                float osc_data = osc_pitch + osc_detune + osc_pw;

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

                cache_osc[oscId].pitch = GetPitchTableValue(osc_pitch);
                cache_osc[oscId].detune = GetDetuneTableValue(osc_detune);   
                cache_osc[oscId].freq_factor = cache_osc[oscId].pitch * cache_osc[oscId].detune;

                cache_osc[oscId].waveform = (paramManager.GetValue(OSC_WAVEFORM[oscId]));
                cache_osc[oscId].pw = osc_pw;
                cache_osc[oscId].active = paramManager.GetValue(OSC_ACTIVE[oscId]);
                cache_osc[oscId].amp = osc_amp;
            }
            polyToMonoSwitch = false;
        }
        dirty.oscParams = false;
        is_osc_dirty = true;
    }
    if (dirty.adsrParams)
    {
        float attack = paramManager.GetValue(P::ADSR_ATTACK);
        float decay = paramManager.GetValue(P::ADSR_DECAY);
        float sustain = paramManager.GetValue(P::ADSR_SUSTAIN);
        float release = paramManager.GetValue(P::ADSR_RELEASE);
        for (size_t v = 0; v < VOICE_NUM; ++v)
        {
            cache_voice[v].adsr_attack = attack;
            cache_voice[v].adsr_decay = decay;
            cache_voice[v].adsr_sustain = sustain;
            cache_voice[v].adsr_release = release;
        }
        dirty.adsrParams = false;
        is_env_dirty = true;
    }
    if (dirty.filterParams) {
        LadderFilter::FilterMode mode = static_cast<LadderFilter::FilterMode>(paramManager.GetValue(P::FILTER_MODE));
        float filter_drive = paramManager.GetValue(P::FILTER_DRIVE);
        float input_drive = 1.0f + (filter_drive * 4.0f);
        for (size_t v = 0; v < VOICE_NUM; ++v)
        {
            cache_voice[v].filter_cutoff = paramManager.GetValue(P::FILTER_CUTOFF);
            cache_voice[v].filter_resonance = paramManager.GetValue(P::FILTER_RESONANCE);
            voice[v].flt.SetFilterMode(mode);
            voice[v].flt.SetPassbandGain(0.5f);
            voice[v].flt.SetInputDrive(input_drive);
        }
        dirty.filterParams = false;
        is_flt_dirty = true;
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
    ModSourcesProcess();

    static float voice_freq[VOICE_NUM];
    static float voice_amp[VOICE_NUM];

    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            if (is_osc_dirty)
            {
                float freq_osc_factor_value = cache_osc[oscId].freq_factor * pitch_bend_multiplier;
                voice_freq[v] = voice[v].freq * freq_osc_factor_value;
                voice_amp[v] = voice[v].vel * cache_osc[oscId].amp;
                float pw_osc_factor_value = cache_osc[oscId].pw;

                voice[v].osc[oscId].SetFreq(voice_freq[v] + (voice_freq[v] * paramManager.GetModifierPerVoice(OSC_FREQ[oscId], v)));
                voice[v].osc[oscId].SetAmp((voice_amp[v] + ((100.0f - voice_amp[v]) * paramManager.GetModifierPerVoice(OSC_AMP[oscId], v))));
                voice[v].osc[oscId].SetPw((pw_osc_factor_value + (100.0f - pw_osc_factor_value) * paramManager.GetModifierPerVoice(OSC_PWM[oscId], v)));

                voice[v].osc[oscId].SetActive(cache_osc[oscId].active);
                voice[v].osc[oscId].SetWaveform(cache_osc[oscId].waveform);
                voice[v].osc[oscId].SetPortamento(cache_global.portamento);
            }
            if (isOscSyncNeeded[oscId] || isModAffectsOscFreq > 0)
            {
                float phase = voice[v].osc[0].GetPhase();
                if (oscId != 0)
                { 
                    voice[v].osc[oscId].SyncPhase(phase);
                }
                if (v == VOICE_NUM - 1) 
                {
                    isOscSyncNeeded[oscId] = false; 
                }
            }
        }
        if (is_flt_dirty)
        {
            float filter_cutoff = cache_voice[v].filter_cutoff;
            float filter_resonance = cache_voice[v].filter_resonance;
            voice[v].flt.SetFreq(filter_cutoff + ((20000.0f - filter_cutoff) * paramManager.GetModifierPerVoice(P::FILTER_CUTOFF, v)));
            voice[v].flt.SetRes(filter_resonance + ((100.0f - filter_resonance) * paramManager.GetModifierPerVoice(P::FILTER_RESONANCE, v)));
        }
        if (is_env_dirty)
        {
            voice[v].adsr.SetAttackTime(cache_voice[v].adsr_attack, 1.0f);
            voice[v].adsr.SetDecayTime(cache_voice[v].adsr_decay);
            voice[v].adsr.SetSustainLevel(cache_voice[v].adsr_sustain);
            voice[v].adsr.SetReleaseTime(cache_voice[v].adsr_release);
        }
    }
    is_voice_dirty = false;
    is_osc_dirty = false;
    is_flt_dirty = false;
    is_env_dirty = false;

    float outL = 0.0f;
    float outR = 0.0f;

    for (size_t v = 0; v < VOICE_NUM; ++v)
    {   
        float voice_out = 0.0f;

        for (size_t oscId = 0; oscId < OSC_NUM; ++oscId)
        {
            voice_out += voice[v].osc[oscId].Process();
        }
        float flt_out = voice[v].flt.Process(voice_out);
        float env_value = voice[v].adsr.Process(voice[v].gate);
        float voice_out_env = flt_out * env_value * inv_voice_num;
        
        outL += voice_out_env * cache_voice[v].pan_correction[0]; 
        outR += voice_out_env * cache_voice[v].pan_correction[1]; 
        
        // if the voice is not active and the envelope is below 0.00001f, kill the voice
        if (!(voice[v].active && voice[v].gate) && env_value <= 0.000001f) 
        {
            voice[v].gate = false;
            voice[v].active = false;
        } 
    }
    isModAffectsOscFreq = 0;
    
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

float softClip(float x)
{
    if (x > 1.0f)  return 1.0f - 1.0f / (x + 1.0f);
    if (x < -1.0f) return -1.0f - 1.0f / (x - 1.0f);
    return x;
}

void InitPanningTable()
{
    for (int i = 0; i < PANNING_TABLE_SIZE; i++) {
        float t = (float)i / (PANNING_TABLE_SIZE - 1);
        panningTable[i][0] = sqrtf(1.0f - t);
        panningTable[i][1] = sqrtf(t);
    }
}

void VoicePanningInit()
{
    for (size_t v = 0; v < VOICE_NUM; ++v)
    {
        // Обчислити фінальну позицію: voice_pan * globalPan
        float panPos = voice_pan_range[v] * cache_global.pan;  // -1..1
        // Конвертувати -1..1 → 0..1 для sqrt
        float panNorm = (panPos + 1.0f) * 0.5f;  // 0..1
        int idx = (int)(panNorm * (PANNING_TABLE_SIZE - 1)); 
        cache_voice[v].pan_correction[0] = panningTable[idx][0];
        cache_voice[v].pan_correction[1] = panningTable[idx][1];
    }
}

void SynthVoiceReset(uint8_t voice_num){
    voice[voice_num].active = false;
    voice[voice_num].gate = false;
    voice[voice_num].note = 0;
    voice[voice_num].freq = 0.0f;
    voice[voice_num].vel = 0.0f;
    voice[voice_num].timestamp = 0;
}
