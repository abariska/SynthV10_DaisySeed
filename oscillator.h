

/*
Copyright (c) 2023 Electrosmith, Corp, GRAME, Centre National de Creation Musicale.

Based on blosc.h source code is governed by the LGPL V2.1
license that can be found in the LICENSE file or at
https://opensource.org/license/lgpl-2-1/
*/

#pragma once
#ifndef OSC_H
#define OSC_H

#include <stdint.h>

/** Band Limited Oscillator

 Based on bltriangle, blsaw, blsquare from soundpipe
 Original Author(s): Paul Batchelor, saw2 Faust by Julius Smith
 Ported by Ben Sergentanis, May 2020
*/
class Osc
{
  public:
    Osc() {}
    ~Osc() {}
    /** Bl Waveforms
*/
    enum Waveforms
    {
        WAVE_SIN,
        WAVE_TRIANGLE,
        WAVE_SAW,
        WAVE_SQUARE,
        WAVE_OFF,
    };


    /** -Initialize oscillator.
        -Defaults to: 440Hz, .5 amplitude, .5 pw, Triangle.
    */
    void Init(float sample_rate);


    /** - Get next floating point oscillator sample.
    */
    float Process();
    
    /** - Float freq: Set oscillator frequency in Hz.
    */
    void SetFreq(float freq);
    /** - Float amp: Set oscillator amplitude, 0 to 1.
    */
    inline void SetAmp(float amp) { amp_ = amp; };
    /** - Float pw: Set square osc pulsewidth, 0 to 1. (no thru 0 at the moment)
    */
    inline void SetPw(float pw) { pw_ = 1.0f - pw; };
    /** - uint8_t waveform: select between waveforms from enum above.
        - i.e. SetWaveform(BL_WAVEFORM_SAW); to set waveform to saw
    */
    inline void SetWaveform(uint8_t waveform) { mode_ = waveform; }

    /** - Resets the phase to 0. */
    void Reset();
    void Sync(float phase, int iota);
    float GetPhase() const;
    int GetIota() const;

  private:
    float freq_, amp_, pw_,
        sampling_freq_, phase_, phase_inc_;
    uint8_t mode_;
    int     iota_;

    // These are no longer used but are kept to match the old structure
    float half_sr_, quarter_sr_, sec_per_sample_, two_over_sr_, four_over_sr_;
};

#endif
