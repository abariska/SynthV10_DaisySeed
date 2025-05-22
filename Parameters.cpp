#include "Parameters.h"
#include "daisysp.h" // Add for using constants

using namespace daisysp;

SynthParams params;
// Parameters initialization - old version replaced with new one in Parameters.h
void InitSynthParams() {
    // Voice template initialization
    params.voice.osc[0].active = true;
    params.voice.osc[1].active = false;
    params.voice.osc[2].active = false;
    for (size_t o = 0; o < OSC_NUM; o++) {
        params.voice.osc[o].waveform = 1.0f;
        params.voice.osc[o].freq = 440.0f;
        params.voice.osc[o].pw = 0.5f;
        params.voice.osc[o].amp = 0.1f;
        params.voice.osc[o].pitch = 0.0f;
        params.voice.osc[o].detune = 0.0f;
    }
    
    params.voice.filter.cutoff = 99.99f;
    params.voice.filter.resonance = 0.0f;
    
    params.voice.adsr.attack = 0.01f;
    params.voice.adsr.decay = 0.1f;
    params.voice.adsr.sustain = 1.0f;
    params.voice.adsr.release = 0.5f;
    params.voice.adsr.retrigger = false;
    
    // Global LFO initialization
    params.lfo.freq = 0.5f;
    params.lfo.depth = 0.0f;
    params.lfo.waveform = 0.0f;

    params.global.isMono = true;
    
    // Effects initialization
    InitEffectParams();
}

void InitEffectParams() {
    // Initialize parameters for each effect block
    for (size_t e = 0; e < 2; e++) {
        // General effect block settings
        params.overdriveParams.isActive = false;

        // Overdrive
        params.overdriveParams.drive = 0.0f;
        params.overdriveParams.isActive = false;
        
        // Chorus
        params.chorusParams.freq = 0.2f;
        params.chorusParams.depth = 0.0f;
        params.chorusParams.delay = 0.0f;
        params.chorusParams.feedback = 0.0f;
        params.chorusParams.pan = 0.5f;
        params.chorusParams.isActive = false;
        
        // Compressor
        params.compressorParams.attack = 0.01f;
        params.compressorParams.release = 0.01f;
        params.compressorParams.threshold = 0.0f;
        params.compressorParams.ratio = 1.0f;
        params.compressorParams.makeup = 0.0f;
        params.compressorParams.isActive = false;
        
        // Reverb
        params.reverbParams.feedback = 0.0f;
        params.reverbParams.dryWet = 0.0f;
        params.reverbParams.isActive = false;
    }
}

// Update parameters
// void UpdateParams(Synth& synth, Effects& effects) {
//     // Do nothing to avoid incomplete type issues
//     // Objects are updated directly from parameters in SynthV10.cpp
// }

// // Stubs for preset save/load functions
// void SavePreset(uint8_t presetNumber) {
//     // Code for saving preset will be here
// }

// void LoadPreset(uint8_t presetNumber) {
//     // Code for loading preset will be here
// } 