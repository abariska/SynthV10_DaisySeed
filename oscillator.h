#pragma once
#ifndef OSC_H
#define OSC_H

#include <stdint.h>

/** Band Limited Oscillator

*/
class Osc
{
public:
    Osc() {}
    ~Osc() {}

    enum Waveforms
    {
        WAVE_SIN,
        WAVE_TRIANGLE,
        WAVE_SAW,
        WAVE_SQUARE,
        WAVE_NOISE,
        WAVE_COUNT,
    };

    uint8_t sampleCounter;

    void Init(float sample_rate, bool is_lfo = false);

    void SetFreq(float freq) { targetFreq = freq; }
    void SetPortamento(float portamento) { freqSlewRate = freqSlewRateBase * powf(0.0001f, portamento);   }

    float Process();
    float GetPhase() const { return phaseOsc; }

    void SetWaveform(int wf) { mode = wf; }
    void SetAmp(float a) { amp = a; }
    void SetPw(float pw_) { pw = pw_; }
    void SyncPhaseToZero() { phaseOsc = 0.0f; }
    void SetDrift(float randomValue);

private:
    float sampleRate;

    float currentFreq;
    float targetFreq;
    float freqSlewRate;
    float freqSlewRateBase;
    float phaseOsc;
    float phaseInc;
    float driftValue;
    float driftTarget;
    float driftSlew;
    float driftAmount;
    uint32_t rng_state;

    int mode;
    float amp;
    float currentAmp;
    float ampSlew;
    float pw;

    float blepGain;
    float prev_phase;
    float current_phase;
    uint32_t noiseState;
    
    bool use_gain;
};

#endif
