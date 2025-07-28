#include "oscillator.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern float g_debug_freq;

// PolyBLEP function to reduce aliasing on harsh waveforms
static inline float poly_blep(float t, float dt)
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    } else if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

void Osc::Init(float sample_rate)
{
    sampling_freq_ = sample_rate;
    freq_ = 440.0f;
    amp_  = 0.5f;
    pw_   = 0.5f;
    mode_ = WAVE_SIN;
    phase_ = 0.0f;
    phase_inc_ = 0.0f;
    pan_ = 0.5f;
}

void Osc::Reset()
{
    phase_ = 0.0f;
}

void Osc::SetFreq(float freq)
{
    freq_ = freq;
    phase_inc_ = freq_ / sampling_freq_;
}

// Sync, GetPhase, GetIota are not part of this simplified version,
// but their definitions can be added if needed for sync logic.
// For now, they are omitted to match the "simple + standalone" state.

float Osc::Process() 
{
    float out = 0.0f;
    
    float current_phase = phase_;

    switch(mode_)
    {
        case WAVE_SIN:
            out = sinf((current_phase + 0.25f) * 2.0f * M_PI); // +0.25f - offset to avoid DC offset
            break;

        case WAVE_TRIANGLE:
            out = 4.0f * (fabsf(current_phase - 0.5f) - 0.25f);
            break;

        case WAVE_SAW:
            out = 1.0f - 2.0f * current_phase;
            out += poly_blep(current_phase, phase_inc_);
            break;
            
        case WAVE_SQUARE:
            out = current_phase < pw_ ? 1.0f : -1.0f;
            out += poly_blep(current_phase, phase_inc_);
            out -= poly_blep(fmodf(current_phase + (1.0f - pw_), 1.0f), phase_inc_);
            break;

        case WAVE_OFF:
        default:
            out = 0.0f;
            break;
    }

    // Increment internal phase
    phase_ += phase_inc_;
    if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
    }
    
    return out * amp_;
}
