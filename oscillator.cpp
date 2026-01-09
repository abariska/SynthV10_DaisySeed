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

inline float clampf(float x, float lo, float hi)
{
    return fminf(hi, fmaxf(lo, x));
}

static constexpr float kWaveGain[] = {
    0.8f, // WAVE_SIN
    0.95f, // WAVE_TRIANGLE
    1.0f, // WAVE_SAW
    0.6f, // WAVE_SQUARE
    0.8f  // WAVE_NOISE (рівномірний)
};

void Osc::Init(float sample_rate , bool is_lfo)
{
    sampleRate = sample_rate;
    targetFreq = 440.0f;
    currentFreq = 440.0f;
    freqSlewRateBase = 0.1f;
    freqSlewRate = 1.0f;
    mode = WAVE_SIN;
    amp = 0.1f;
    pw = 0.5f;
    noiseState = 1;
    driftTarget = 0.0f;
    driftSlew = 0.042f;
    driftAmount = 0.0f; 
    driftValue = 0.0f;

    phaseOsc = 0.0f;
    blepGain = 1.0f;

    currentAmp = 0.0f;
    ampSlew = 0.1f;

    use_gain = !is_lfo;
    sampleCounter = 0;
    need_phase_sync = false;
}

void Osc::SetDrift(float randomValue)
{
    driftTarget = randomValue;
}

void Osc::SetDriftAmount(float amount)
{
    driftAmount = amount;
}

void Osc::ResetDrift()
{
    driftTarget = 0.0f;
    need_phase_sync = true;
}

void Osc::SyncPhaseToZero()
{
    need_phase_sync = true;
}

float Osc::Process()
{
    float out = 0.0f;
    float gain = kWaveGain[mode] * 0.5f;
    sampleCounter++;

    driftValue += (driftTarget - driftValue) * driftSlew; 

    baseFreq += (targetFreq - baseFreq) * freqSlewRate; 
    phaseBaseInc = baseFreq / sampleRate; 

    float driftInc = (baseFreq + driftValue * driftAmount) * SAMPLE_TIME;
    
    phaseBase += phaseBaseInc;
    phaseBase -= (phaseBase >= 1.0f) ? 1.0f : 0.0f;
    phaseOsc += driftInc;
    phaseOsc -= (phaseOsc >= 1.0f) ? 1.0f : 0.0f;

    phaseDiff = phaseOsc - phaseBase;

    if (need_phase_sync)
    {
        if (fabsf(phaseDiff) < 0.00000001f)
        {
            phaseOsc = phaseBase;
            need_phase_sync = false;
        }
        else
        {
            phaseOsc -= phaseDiff * driftSlew;
            phaseOsc -= (phaseOsc >= 1.0f) ? 1.0f : 0.0f;
        }
    }

    currentAmp += (amp - currentAmp) * 0.1f;

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
    case WAVE_NOISE:
        noiseState = noiseState * 1664525U + 1013904223U;
        out = (noiseState >> 8) * (1.0f / 8388608.0f);
        break;
    default:
        out = 0.0f;
        break;
    }

    gain = use_gain ? gain * currentAmp : 1.0f;
    return out * gain;
}

