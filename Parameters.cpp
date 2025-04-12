#include "Parameters.h"
#include "daisysp.h" // Add for using constants

using namespace daisysp;

SynthParams params;
EffectUnitParams effectParams;


// Parameters initialization - old version replaced with new one in Parameters.h
void InitSynthParams() {
    // Voice template initialization
    params.voice.osc[0].active = true;
    params.voice.osc[1].active = false;
    params.voice.osc[2].active = false;
    for (int o = 0; o < OSC_NUM; o++) {
        params.voice.osc[o].waveform = 0;
        params.voice.osc[o].freq = 440.0f;
        params.voice.osc[o].pw = 0.5f;
        params.voice.osc[o].amp = 0.1f;
        params.voice.osc[o].pitch = 0;
        params.voice.osc[o].detune = 0;
    }
    
    params.voice.filter.cutoff = 1000.0f;
    params.voice.filter.resonance = 0.0f;
    
    params.voice.adsr.attack = 0.01f;
    params.voice.adsr.decay = 0.1f;
    params.voice.adsr.sustain = 1.0f;
    params.voice.adsr.release = 0.5f;
    params.voice.adsr.retrigger = false;
    
    // Global LFO initialization
    params.lfo.freq = 0.5f;
    params.lfo.amp = 0.0f;
    params.lfo.waveform = 0;
    params.lfo.pw = 0.5f;

    params.global.isMono = false;
    params.global.isUnison = false;
    params.global.tuning = 0.0f;
    
    // Effects initialization
    InitEffectParams();
}

void InitEffectParams() {
    // Initialize parameters for each effect block
    for (int e = 0; e < 2; e++) {
        // General effect block settings
        params.effectUnits[e].isActive = false;
        params.effectUnits[e].activeEffect = EFFECT_NONE;
        
        // Overdrive
        params.effectUnits[e].overdrive.drive = 0.0f;
        params.effectUnits[e].overdrive.isActive = false;
        
        // Chorus
        params.effectUnits[e].chorus.lfoFreq = 0.2f;
        params.effectUnits[e].chorus.lfoDepth = 0.0f;
        params.effectUnits[e].chorus.delay = 0.0f;
        params.effectUnits[e].chorus.feedback = 0.0f;
        params.effectUnits[e].chorus.pan = 0.5f;
        params.effectUnits[e].chorus.isActive = false;
        
        // Compressor
        params.effectUnits[e].compressor.attack = 0.01f;
        params.effectUnits[e].compressor.release = 0.01f;
        params.effectUnits[e].compressor.threshold = 0.0f;
        params.effectUnits[e].compressor.ratio = 1.0f;
        params.effectUnits[e].compressor.makeup = 0.0f;
        params.effectUnits[e].compressor.isActive = false;
        
        // Reverb
        params.effectUnits[e].reverb.feedback = 0.0f;
        params.effectUnits[e].reverb.dryWet = 0.0f;
        params.effectUnits[e].reverb.isActive = false;
    }
}

// Function to set active effect
void SetActiveEffect(int unitIndex, EffectType effectType) {
    // Make sure index is within valid range
    if (unitIndex < 0 || unitIndex >= 2) return;
    
    // Set active effect
    params.effectUnits[unitIndex].activeEffect = effectType;
    
    // Update individual effect active states in params structure
    // This ensures consistency between activeEffect and individual isActive flags
    params.effectUnits[unitIndex].overdrive.isActive = (effectType == EFFECT_OVERDRIVE);
    params.effectUnits[unitIndex].chorus.isActive = (effectType == EFFECT_CHORUS);
    params.effectUnits[unitIndex].compressor.isActive = (effectType == EFFECT_COMPRESSOR);
    params.effectUnits[unitIndex].reverb.isActive = (effectType == EFFECT_REVERB);
}


// // Stubs for preset save/load functions
// void SavePreset(uint8_t presetNumber) {
//     // Code for saving preset will be here
// }

// void LoadPreset(uint8_t presetNumber) {
//     // Code for loading preset will be here
// } 