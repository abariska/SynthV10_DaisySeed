#pragma once
#ifndef PHASE_GENERATOR_H
#define PHASE_GENERATOR_H

class PhaseGenerator
{
  public:
    PhaseGenerator() {}
    ~PhaseGenerator() {}

    /** Initializes the PhaseGenerator.
        \param sample_rate The audio sample rate.
    */
    void Init(float sample_rate);

    /** Processes the next step of the phase, incrementing it.
        \return The current phase value (0.0 to 1.0).
    */
    float Process();

    /** Sets the frequency of the phase generator.
        \param freq Frequency in Hz.
    */
    void SetFreq(float freq);

    /** Resets the phase to 0. */
    void Reset();
    
    /** Gets the current phase without incrementing.
        \return The current phase value.
    */
    float GetPhase() const;


  private:
    float sample_rate_;
    float freq_;
    float phase_;
    float phase_inc_;
};

#endif 