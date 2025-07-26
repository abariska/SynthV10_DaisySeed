#include "phase_generator.h"

void PhaseGenerator::Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    freq_ = 440.0f;
    phase_ = 0.0f;
    phase_inc_ = 0.0f;
}

float PhaseGenerator::Process()
{
    // Store current phase to return it
    float current_phase = phase_;
    
    // Increment phase for the next call
    phase_ += phase_inc_;
    if (phase_ >= 1.0f)
    {
        phase_ -= 1.0f;
    }
    
    return current_phase;
}

void PhaseGenerator::SetFreq(float freq)
{
    freq_ = freq;
    // Recalculate the phase increment based on the new frequency
    phase_inc_ = freq_ / sample_rate_;
}        

void PhaseGenerator::Reset()
{
    phase_ = 0.0f;
}

float PhaseGenerator::GetPhase() const
{
    return phase_;
} 