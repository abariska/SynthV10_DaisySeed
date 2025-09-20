#include "oscillator.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern float g_debug_freq;

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

void Osc::Init(float sample_rate)
{
    sampleRate = sample_rate;
    targetFreq = 440.0f;
    currentFreq = 440.0f;
    slewRate = 0.2f;
    mode = WAVE_SIN;
    amp = 1.0f;
    pw = 0.5f;

    phaseOsc = 0.0f;
    phaseOffset = 0.0f;
    blepGain = 1.0f;

    UpdateIncrement();
}

float Osc::Process()
{

    if (fabs(targetFreq - currentFreq) > 0.1f)
    {
        currentFreq += (targetFreq - currentFreq) * slewRate;
    }
    UpdateIncrement();

    phaseOsc += phaseInc;
    phaseOsc -= (phaseOsc >= 1.0f) ? 1.0f : 0.0f;

    float out = 0.0f;

    switch (mode)
    {
    case WAVE_SIN:
        out = sinf((phaseOsc + 0.25f) * 2.0f * M_PI); // +0.25f - offset to avoid DC offset
        break;
    case WAVE_TRIANGLE:
        out = 4.0f * (fabsf(phaseOsc - 0.5f) - 0.25f);
        break;
    case WAVE_SAW:
        out = 1.0f - 2.0f * phaseOsc;
        out += poly_blep(phaseOsc, phaseInc) * blepGain;
        break;
    case WAVE_SQUARE:
        out = phaseOsc < pw ? 1.0f : -1.0f;
        out += poly_blep(phaseOsc, phaseInc) * blepGain;
        out -= poly_blep(fmodf(phaseOsc + (1.0f - pw), 1.0f), phaseInc) * blepGain;
        break;
    default:
        out = 0.0f;
        break;
    }

    return out * amp;
}

void Osc::UpdateIncrement()
{
    phaseInc = currentFreq / sampleRate;
}
