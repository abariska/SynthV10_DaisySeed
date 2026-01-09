#pragma once
#ifndef OSC_H
#define OSC_H

#include <stdint.h>

#define SAMPLE_RATE 48000.0f
#define SAMPLE_TIME 1.0f / SAMPLE_RATE

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

    uint16_t sampleCounter;
    bool need_phase_sync;

    void Init(float sample_rate, bool is_lfo = false);

    void SetFreq(float freq) { targetFreq = freq; }
    void SetWaveform(int wf) { mode = wf; }
    void SetAmp(float a) { amp = a; }
    void SetPw(float pw_) { pw = pw_; }
    void SyncPhase(float phase) { phaseOsc = phase; }
    void SyncPhaseToZero();
    void SetDrift(float randomValue);
    void SetDriftAmount(float amount);
    void SetPortamento(float portamento) { freqSlewRate = freqSlewRateBase * powf(0.0001f, portamento);   }
    void ResetDrift();

    float Process();
    float GetPhase() const { return phaseOsc; }
    float GetAmp() const { return amp; }
    float GetFreq() const { return currentFreq; }
    float GetPw() const { return pw; }
    float GetWaveform() const { return mode; }
    float GetDrift() const { return driftValue; }
    float GetDriftTarget() const { return driftTarget; }
    float GetDriftSlew() const { return driftSlew; }
    float GetDriftAmount() const { return driftAmount; }
    float GetSampleCounter() const { return sampleCounter; }
    float GetPhaseOsc() const { return phaseOsc; }

private:
    float sampleRate;

    float currentFreq;
    float baseFreq;
    float targetFreq;
    float freqSlewRate;
    float freqSlewRateBase;
    float phaseOsc;
    float phaseBaseInc;
    float phaseIncWithDrift;
    float phaseDiff;
    float phaseBase;
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
    float current_phase;
    uint32_t noiseState;
    
    bool use_gain;
};

#endif
