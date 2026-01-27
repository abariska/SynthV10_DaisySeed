#include "oscillator.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// PolyBLEP function to reduce aliasing on harsh waveforms
static inline float poly_blep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

static constexpr float kWaveGain[] = {
    0.8f, // WAVE_SIN
    0.95f, // WAVE_TRIANGLE
    1.0f, // WAVE_SAW
    0.6f, // WAVE_SQUARE
    0.8f  // WAVE_NOISE (рівномірний)
};

// Osc
void Osc::Init(float sample_rate)
{
    sampleRate = sample_rate;
    targetFreq = 440.0f;
    currentFreq = 440.0f;
    slewRate = 1.0f;
    slewRateBase = 0.1f; 
    mode = SIN;
    pw = 0.5f;
    noiseState = 1;

    phaseOsc = 0.0f;
    phaseOffset = 0.0f;
    blepGain = 1.0f;
    currentAmp = 0.1f;
    targetAmp = 0.0f;
    active = false;
}

float Osc::Process()
{
    currentFreq += (targetFreq != currentFreq) * (targetFreq - currentFreq) * slewRate; 
    phaseInc = currentFreq / sampleRate;

    if (!active)
    {
        targetAmp = 0.0f;
    }
    currentAmp += (targetAmp - currentAmp) * 0.1f;
    phaseOsc += phaseInc;
    phaseOsc -= (phaseOsc >= 1.0f) ? 1.0f : 0.0f;

    float out = 0.0f;
    float gain = kWaveGain[mode] * 0.5f;

    switch (mode)
    {
    case SIN:
        out = sinf((phaseOsc + 0.25f) * 2.0f * M_PI); // +0.25f - offset to avoid DC offset
        break;
    case TRIANGLE:
        out = 4.0f * (fabsf(phaseOsc - 0.5f) - 0.25f);
        break;
    case SAW:
        out = 1.0f - 2.0f * phaseOsc;
        out += poly_blep(phaseOsc, phaseInc) * blepGain;
        break;
    case PULSE:
        out = phaseOsc < pw ? 1.0f : -1.0f;
        out += poly_blep(phaseOsc, phaseInc) * blepGain;
        out -= poly_blep(fmodf(phaseOsc + (1.0f - pw), 1.0f), phaseInc) * blepGain;
        break;
    case NOISE:
        noiseState = noiseState * 1664525U + 1013904223U;
        out = (int32_t(noiseState)) / 2147483648.0f;
        break;
    default:
        out = 0.0f;
        break;
    }
    prev_phase = phaseOsc;

    gain = gain * currentAmp;
    return out * gain;
}

// Osc Lfo
void OscLfo::Init(float sample_rate)
{
    sampleRate = sample_rate;
    freq = 1.0f;
    amp = 1.0f;
    wave = SIN;
    phase = 0.0f;
    timer = 0;
    noiseState = 1;
}

float OscLfo::Process()
{
    phaseInc = freq / sampleRate;
    phase += phaseInc;
    bool phaseExceeded = (phase >= 1.0f);
    if (phaseExceeded)
    {
        phase = 0.0f;
    }

    float out = 0.0f;
    switch (wave)
    {
    case SIN:
        out = (sinf((phase + 0.25f) * 2.0f * M_PI) + 1.0f) * 0.5f;
        break;
    case TRIANGLE:
        out = phase < 0.5f ? 2.0f * phase : 2.0f * (1.0f - phase);
        break;
    case SAW:
        out = 1.0f - phase;
        break;
    case RAMP:
        out = phase;
        break;
    case PULSE:
        out = phase < 0.5f ? 0.0f : 1.0f;
        break;
    case NOISE:
        if (phaseExceeded) 
        {
            noiseState = noiseState * 1664525U + 1013904223U;
            noiseValue = ((noiseState >> 1) / 2147483648.0f); 
        }
        out = noiseValue;
        break;
    default:
        out = 0.0f;
        break;
    }
    return out * amp;
}