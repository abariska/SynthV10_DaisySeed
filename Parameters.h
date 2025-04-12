#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisysp.h"

// Required for array structures
#define OSC_NUM 3

using namespace daisysp;

// Enumeration of effect types
enum EffectType {
    EFFECT_NONE,
    EFFECT_OVERDRIVE,
    EFFECT_CHORUS,
    EFFECT_COMPRESSOR,
    EFFECT_REVERB
};

// Structure for Overdrive effect parameters
struct OverdriveParams {
    float drive;          // Drive level
    bool isActive;        // Is the effect active
};

// Structure for Chorus effect parameters
struct ChorusParams {
    float lfoFreq;        // LFO frequency
    float lfoDepth;       // LFO depth
    float delay;          // Delay
    float feedback;       // Feedback
    float pan;            // Pan
    bool isActive;        // Is the effect active
};

// Structure for Compressor effect parameters
struct CompressorParams {
    float attack;         // Attack time
    float release;        // Release time
    float threshold;      // Threshold
    float ratio;          // Ratio
    float makeup;         // Makeup gain
    bool isActive;        // Is the effect active
};

// Structure for Reverb effect parameters
struct ReverbParams {
    float dryWet;         // Dry/Wet balance
    float feedback;       // Feedback
    float lpFreq;         // Low-pass filter frequency
    bool isActive;        // Is the effect active
};

// Structure for effect unit parameters
struct EffectUnitParams {
    OverdriveParams overdrive;     // Overdrive parameters
    ChorusParams chorus;           // Chorus parameters
    CompressorParams compressor;   // Compressor parameters
    ReverbParams reverb;           // Reverb parameters
    bool isActive;                 // Is the effect unit active
    EffectType activeEffect;       // Currently active effect
};

// Structure for storing synthesizer parameters
struct SynthParams {
    // Single voice template with all settings
    struct {
        struct {
            int waveform;
            float pw;
            float amp;
            int pitch;
            float detune;
            bool active;
            float freq; 
            float pan; 
        } osc[OSC_NUM];
        
        struct {
            float cutoff;
            float resonance;
        } filter;
        
        struct {
            float attack;
            float decay;
            float sustain;
            float release;
            bool retrigger;
        } adsr;
    } voice;  // Single voice template
    
    // Global LFO
    struct {
        float freq;
        float amp;
        int waveform;
        float pw;
    } lfo;

    struct {
        float tuning;
        bool isMono;
        bool isUnison;
    } global;

    EffectUnitParams effectUnits[2];
};

// Global variable for accessing parameters
extern SynthParams params;
extern EffectUnitParams effectParams;

// Functions for initializing parameters
void InitSynthParams();
void InitEffectParams();

// Functions for updating parameters
void UpdateParams(SynthParams& synthParams, EffectUnitParams& effectParams, size_t voiceIndex);

// Function to set active effect
void SetActiveEffect(int unitIndex, EffectType effectType);

// Function to get effect name as a string
inline const char* GetEffectName(EffectType effect) {
    switch (effect) {
        case EFFECT_NONE:        return " - ";
        case EFFECT_OVERDRIVE:   return "Overdrive";
        case EFFECT_CHORUS:      return "Chorus";
        case EFFECT_COMPRESSOR:  return "Compressor";
        case EFFECT_REVERB:      return "Reverb";
        default:          return "Unknown";
    }
}

// Function to toggle effect unit active state
inline void ToggleEffectUnitActive(int unitIndex) {
    if (unitIndex >= 0 && unitIndex < 2) {
        params.effectUnits[unitIndex].isActive = !params.effectUnits[unitIndex].isActive;
    }
}

// Functions for saving/loading presets
void SavePreset(uint8_t presetNumber);
void LoadPreset(uint8_t presetNumber);

#endif // PARAMETERS_H
