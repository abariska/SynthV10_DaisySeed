#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisysp.h"

// Required for array structures
#define OSC_NUM 3

using namespace daisysp;

enum Waves {
    TRI,
    SAW,
    SQR,
    OFF
};

enum ValueType {
    REGULAR,
    X100,
    WAVEFORM
};

// Structure for storing synthesizer parameters
struct SynthParams {
    // Single voice template with all settings
    struct {
        struct {
            float waveform;
            float pw;
            float amp;
            float pitch;
            float detune;
            float freq; 
            float pan; 
            bool active;
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
        float depth;
        float waveform;
    } lfo;

    struct {
        bool isMono;
    } global;

    // Structure for Overdrive effect parameters
    struct {
        float drive;          // Drive level
        bool isActive;        // Is the effect active
    } overdriveParams;

    // Structure for Chorus effect parameters
    struct {
        float freq;        // LFO frequency
        float depth;       // LFO depth
        float delay;          // Delay
        float feedback;       // Feedback
        float pan;            // Pan
        bool isActive;        // Is the effect active
    } chorusParams;

    // Structure for Compressor effect parameters
    struct {
        float attack;         // Attack time
        float release;        // Release time
        float threshold;      // Threshold
        float ratio;          // Ratio
        float makeup;         // Makeup gain
        bool isActive;        // Is the effect active
    } compressorParams;

    // Structure for Reverb effect parameters
    struct {
        float dryWet;         // Dry/Wet balance
        float feedback;       // Feedback
        float lpFreq;         // Low-pass filter frequency
        bool isActive;        // Is the effect active
    } reverbParams;

};

// Global variable for accessing parameters
extern SynthParams params;

// Functions for initializing parameters
void InitSynthParams();
void InitEffectParams();

// Functions for saving/loading presets
// void SavePreset(uint8_t presetNumber);
// void LoadPreset(uint8_t presetNumber);

#endif // PARAMETERS_H
