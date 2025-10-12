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

    void PhaseProcess();
    float Process();
    float GetCurrentFreq() const { return currentFreq; }

    void SetWaveform(int wf) { mode = wf; }
    void SetAmp(float a) { amp = a; }
    void SetPw(float pw_) { pw = pw_; }
    void SyncToMaster(float masterPhase) { phaseOsc = masterPhase; }
    void SyncToZero() { phaseOsc = 0.0f; }
    void UpdateIncrement();

private:
    float sampleRate;

    float currentFreq;
    float targetFreq;
    float slewRate;

    float phaseOffset;
    float phaseInc;

    int mode;
    float amp;
    float pw;
    float phaseOsc;
    float blepGain;
    float prev_phase;
    float current_phase;
    uint32_t noiseState;
    float currentAmp;
    bool use_gain;
};

#endif
