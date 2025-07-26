#include "parameters.h"
#include "daisysp.h" // Add for using constants

using namespace daisysp;

SynthParams params;
// Parameters initialization - old version replaced with new one in Parameters.h
void InitSynthParams() {
    // Voice template initialization
    params.osc[0].active = true;
    params.osc[1].active = true;
    // params.osc[2].active = false;
    for (size_t o = 0; o < OSC_NUM; o++) {
        params.osc[o].waveform = 1.0f;
        params.osc[o].freq = 440.0f;
        params.osc[o].pw = 0.5f;
        params.osc[o].amp = 0.5f;
        params.osc[o].pitch = 0.0f;
        params.osc[o].detune = 0.0f;
    }
    
    params.filter.cutoff = 5000.0f;
    params.filter.resonance = 0.0f;
    
    params.adsr.attack = 0.01f;
    params.adsr.decay = 0.1f;
    params.adsr.sustain = 1.0f;
    params.adsr.release = 0.5f;
    params.adsr.retrigger = false;
    
    // Global LFO initialization
    params.lfo.freq = 0.5f;
    params.lfo.depth = 0.0f;
    params.lfo.waveform = 2.0f;

    params.global.isMono = true;
    params.global.isLegato = false;
    params.global.portamentoTime = 0.0f;
    params.global.analogAmount = 0.7f;  // 70% аналогового характеру за замовчуванням
    
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
        params.chorusParams.isActive = false;
        
        // Compressor
        params.compressorParams.attack = 0.01f;
        params.compressorParams.release = 0.01f;
        params.compressorParams.threshold = 0.0f;
        params.compressorParams.ratio = 1.0f;
        // params.compressorParams.makeup = 0.0f;
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