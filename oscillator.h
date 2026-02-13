#pragma once
#ifndef OSC_H
#define OSC_H

#include <stdint.h>

/** Band Limited Oscillator

*/
enum OscWaveforms
{
    SIN,
    TRIANGLE,
    SAW_DOWN,
    SAW_UP,
    PULSE,
    NOISE,
    WAVE_COUNT,
};

class Osc
{
public:
    Osc() {}
    ~Osc() {}

    void Init(float sample_rate);

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
    OscWaveforms wave;
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
    bool active;
};

class OscLfo
{
public:
    OscLfo() {}
    ~OscLfo() {}

    void Init(float sample_rate);

    void SetFreq(float frequency) { freq = frequency; }
    void SetAmp(float amplitude) { amp = amplitude; }
    void SetWaveform(int wf) { wave = static_cast<OscWaveforms>(wf); }
    void SyncPhaseToStart() { phase = 1.0f; }
    float Process();

private:
    float sampleRate;
    float freq;
    float phase;
    float phaseInc;
    float amp;
    uint32_t noiseState;
    float noiseValue;
    OscWaveforms wave;
    uint32_t timer;
};
#endif
