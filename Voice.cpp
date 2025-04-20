#include "Voice.h"

// Definition of global variables
Oscillator lfo;
Adsr mainADSR;

VoiceUnit voice[NUM_VOICES];

// Function definitions
void InitLfo(float samplerate) {
    lfo.Init(samplerate);
}

float ProcessLfo() {
    // Apply parameters from template
    lfo.SetFreq(params.lfo.freq);
    lfo.SetWaveform(params.lfo.waveform);
    lfo.SetAmp(params.lfo.depth);
    return lfo.Process();
}

// Implementation of VoiceUnit methods
void VoiceUnit::Init(float samplerate, int blocksize) {
    isVoiceActive = false; 
    isGated = false;
    note = -1;
    velocity = 0;
    timestamp = 0;

    // Initialize oscillators
    for (size_t i = 0; i < OSC_NUM; i++) {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);

}

void HandleNoteOn(uint8_t midi_note, uint8_t midi_vel) {
    static uint32_t global_time = 0;
    uint32_t current_time = global_time++;
    
    if (params.global.isMono) {
        voice[0].NoteOn(midi_note, midi_vel, current_time);
    } else {
        // Find a free voice
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (!voice[i].isVoiceActive) {
                voice[i].NoteOn(midi_note, midi_vel, current_time);   
                return;
            }
        }
        // If none are free, find the oldest
        uint8_t oldest_index = 0;
        uint32_t oldest_timestamp = UINT32_MAX;
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (voice[i].isVoiceActive && voice[i].timestamp < oldest_timestamp) {
                oldest_timestamp = voice[i].timestamp;
                oldest_index = i;
                voice[oldest_index].isGated = false;
            }
        }
        voice[oldest_index].NoteOn(midi_note, midi_vel, current_time);

    }
}

void HandleNoteOff(uint8_t midi_note) {
    if (params.global.isMono) {
        // In mono mode, check if it's the same note that's currently active
        if (voice[0].isVoiceActive && voice[0].note == midi_note) {
            voice[0].NoteOff();
        }
    } else {
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (voice[i].isVoiceActive && voice[i].note == midi_note) {
                voice[i].NoteOff();
            }
        }
    }
}
void VoiceUnit::NoteOn(uint8_t midi_note, uint8_t midi_vel, uint32_t time){

        note = midi_note;
        velocity = midi_vel;
        timestamp = time;
        isVoiceActive = true;
        isGated = true;
        adsrMain.Retrigger(true);
    }

void VoiceUnit::NoteOff(){
    isGated = false; 
    // isVoiceActive = false; 
}

float VoiceUnit::CalculateFrequency(uint8_t midi_note, int pitch, int detune) {
    // MIDI note 69 (A4) corresponds to 440 Hz
    // Add semitone shift (each semitone is 1 MIDI note)
    float freq = 440.0f * pow(2.0f, ((midi_note + pitch) - 69) / 12.0f);
    freq *= pow(2.0f, (detune));  // detune in cents
    
    return freq;
}

float VoiceUnit::CalculateVelocity(uint8_t velocity, uint8_t midi_note){

    float velocity_factor = velocity / 127.0f;
    float freq_compensation = powf(2.0f, (midi_note - 60.0f) / 48.0f);
    return velocity_factor * freq_compensation;
}

float VoiceUnit::Process(){
    if (!isVoiceActive && !isGated) return 0.0f;

    float sig = 0.0f;

    // Update oscillator parameters
    for (size_t i = 0; i < OSC_NUM; i++) {
        // Update parameters from template
        osc[i].SetWaveform(params.voice.osc[i].waveform);
        osc[i].SetPw(params.voice.osc[i].pw);
        
        // Calculate final frequency
        float final_freq = CalculateFrequency(
            note,
            params.voice.osc[i].pitch,
            params.voice.osc[i].detune
        );
        osc[i].SetFreq(final_freq);
        osc[i].SetAmp(params.voice.osc[i].amp * CalculateVelocity(velocity, note));
        
        // Process only active oscillators
        if (params.voice.osc[i].active) {
            sig += osc[i].Process();
        }
    }
    sig /= OSC_NUM;
    
    flt.SetFreq(params.voice.filter.cutoff);
    flt.SetRes(params.voice.filter.resonance);
    sig = flt.Process(sig);
    
    // Apply ADSR
    adsrMain.SetAttackTime(params.voice.adsr.attack);
    adsrMain.SetDecayTime(params.voice.adsr.decay);
    adsrMain.SetSustainLevel(params.voice.adsr.sustain);
    adsrMain.SetReleaseTime(params.voice.adsr.release);

    if (isGated && params.voice.adsr.retrigger) {
        adsrMain.Retrigger(true);
    }
    
    float env = adsrMain.Process(isGated);
    sig *= env;

        return sig;
}

