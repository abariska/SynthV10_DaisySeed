#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisysp.h"

// Required for array structures
#define OSC_NUM 3
#define PARAM_NAME_LENGTH 8

using namespace daisysp;

enum class ParamUnitName {
    OSC_WAVEFORM_1,
    OSC_PITCH_1,
    OSC_DETUNE_1,
    OSC_AMP_1,
    OSC_PAN_1,
    OSC_WAVEFORM_2,
    OSC_PITCH_2,
    OSC_DETUNE_2,
    OSC_AMP_2,
    OSC_PAN_2,
    OSC_WAVEFORM_3,
    OSC_PITCH_3,
    OSC_DETUNE_3,
    OSC_AMP_3,
    OSC_PAN_3,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    LFO_WAVEFORM,
    LFO_FREQ,
    LFO_DEPTH,
    EFFECT_OVERDRIVE_DRIVE,
    EFFECT_CHORUS_FREQ,
    EFFECT_CHORUS_DEPTH,
    EFFECT_CHORUS_FBK,
    EFFECT_CHORUS_DELAY,
    EFFECT_COMPRESSOR_ATTACK,
    EFFECT_COMPRESSOR_RELEASE,
    EFFECT_COMPRESSOR_THRESHOLD,
    EFFECT_COMPRESSOR_RATIO,
    EFFECT_REVERB_DRYWET,
    EFFECT_REVERB_FBK,
    EFFECT_REVERB_LPFREQ,
    NONE,
    NUM_OF_PARAMS
};

float parameters[static_cast<int>(ParamUnitName::NUM_OF_PARAMS)];

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

enum class Curve {
    LINEAR,
    LOGARITHMIC,
    EXPONENTIAL
};

class DiscreteParameter {
    private:
        float norm_value = 0;
        int physical_value;
        int max_numbers;
        const char* name_lable;
        int param_index;
        float* param_array;
    
    public:
    
        DiscreteParameter(int init_value, int max_vals, const char* lable, uint8_t index, float* array);
    
        void SetNormalized(float n);
    
        void SetPhysical_value(int n);
    
        int AdjustByEncoder(int encoder_inc);
    
        float GetNormalised() const;

        int GetInt() const;
    };
    
class Parameter {
    private:
        float physical_value;
        float norm_value;
        float min;
        float max;
        const char* name_lable;
        int param_index;
        float* param_array;
        Curve curve;
        
    public:
    
    Parameter(float init_value, float min_value, float max_value, const char* lable, 
        uint8_t index, float* array, Curve defaultCurve = Curve::LINEAR);

        float SetNormalized(float n);
    
        float SetPhysicalValue(float n);
        
        float AdjustByEncoder(int encoder_inc);
    
        float GetFloat() const ;
        
        int GetInt() const ;
        
        bool GetBool() const ;
    };

// Structure for storing synthesizer parameters
struct SynthParams {
    // Single voice template with all settings
    struct {
        float waveform;
        float pw;
        float amp;
        float pitch;
        float detune;
        float freq; 
        float pan; 
        float active;
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
        float retrigger;
    } adsr;
    
    // Global LFO
    struct {
        float freq;
        float depth;
        float waveform;
    } lfo;

    struct {
        float isMono;
        float isLegato;
        float portamentoTime;
        float analogAmount;  // 0.0 = цифровий, 1.0 = повністю аналоговий характер
    } global;

    // Structure for Overdrive effect parameters
    struct {
        float drive;          // Drive level
        float isActive;        // Is the effect active
    } overdriveParams;

    // Structure for Chorus effect parameters
    struct {
        float freq;        // LFO frequency
        float depth;       // LFO depth
        float delay;          // Delay
        float feedback;       // Feedback
        float isActive;        // Is the effect active
    } chorusParams;

    // Structure for Compressor effect parameters
    struct {
        float attack;         // Attack time
        float release;        // Release time
        float threshold;      // Threshold
        float ratio;          // Ratio
        float makeup;         // Makeup gain
        float isActive;        // Is the effect active
    } compressorParams;

    // Structure for Reverb effect parameters
    struct {
        float dryWet;         // Dry/Wet balance
        float feedback;       // Feedback
        float lpFreq;         // Low-pass filter frequency
        float isActive;        // Is the effect active
    } reverbParams;

    float none;
};

extern SynthParams params;



// Functions for initializing parameters
void InitSynthParams();
void InitEffectParams();

// Functions for saving/loading presets
// void SavePreset(uint8_t presetNumber);
// void LoadPreset(uint8_t presetNumber);


#endif // PARAMETERS_H
