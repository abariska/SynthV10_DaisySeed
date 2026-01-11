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

    void Init(float sample_rate, bool is_lfo = false);

    void SetPhaseOffset(float offset) { phaseOffset = offset; }
    void SetFreq(float freq) { targetFreq = freq; }
    void SetPortamento(float portamento) { slewRate = slewRateBase * powf(0.0001f, portamento);   }

    float Process();
    float GetCurrentFreq() const { return currentFreq; }
    float GetPhase() const { return phaseOsc; }
    bool IsActive() const { return active; }

    void SetWaveform(int wf) { mode = wf; }
    void SetAmp(float a) { targetAmp = a; }
    void SetPw(float pw_) { pw = pw_; }
    void SyncPhase(float phase) { phaseOsc = phase; }
    void SyncPhaseToZero() { phaseOsc = 0.0f; }
    void SetActive(bool a) { active = a; }

private:
    float sampleRate;

    float currentFreq;
    float targetFreq;
    float slewRate;
    float slewRateBase;

    float phaseOffset;
    float phaseInc;

    int mode;
    float pw;
    float phaseOsc;
    float blepGain;
    float prev_phase;
    float current_phase;
    uint32_t noiseState;
    float currentAmp;
    float targetAmp;
    bool use_gain;
    bool active;
};

#endif
