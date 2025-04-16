#include "Voice.h"

// Definition of global variables
BlOsc lfo;
float lfoValue = 0.0f;
Adsr mainADSR;
VoiceUnit voice;

// Function definitions
void InitLfo(float samplerate) {
    lfo.Init(samplerate);
}

void ProcessLfo() {
    // Apply parameters from template
    lfo.SetFreq(params.lfo.freq);
    lfo.SetWaveform(params.lfo.waveform);
    lfo.SetAmp(params.lfo.amp);
    lfo.SetPw(params.lfo.pw);
    
    // Process LFO and store value in global variable
    lfoValue = lfo.Process();
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
    
    voice.NoteOn(midi_note, midi_vel, current_time);
}

void HandleNoteOff(uint8_t midi_note) {
    if (voice.isVoiceActive && voice.note == midi_note) {
        voice.NoteOff();
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

float VoiceUnit::CalculateVelocity(uint8_t velocity){

    return velocity / 127.0f;
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
        osc[i].SetAmp(params.voice.osc[i].amp * CalculateVelocity(velocity));
        
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
    adsrMain.SetAttackTime(params.voice.adsr.attack, 0.1f);
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



