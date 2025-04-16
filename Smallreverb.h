#pragma once
#ifndef SMALLREVERB_H
#define SMALLREVERB_H

#define DSY_SMALLREVERB_MAX_SIZE 16384 // Reduced buffer size

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace daisysp
{
/**Delay line for internal reverb use
*/
typedef struct
{
    int    write_pos;         /**< write position */
    int    buffer_size;       /**< buffer size */
    int    read_pos;          /**< read position */
    int    read_pos_frac;     /**< fractional component of read pos */
    int    read_pos_frac_inc; /**< increment for fractional */
    int    dummy;             /**<  dummy var */
    int    seed_val;          /**< randseed */
    int    rand_line_cnt;     /**< number of random lines */
    float  filter_state;      /**< state of filter */
    float *buf;               /**< buffer ptr */
} SmallReverbDl;

/** Simplified Stereo Reverb with smaller buffer */
class SmallReverb
{
  public:
    SmallReverb() {}
    ~SmallReverb() {}

    /** Initialize reverb module
        \param sample_rate Audio engine sample rate
    */
    void Init(float sample_rate)
    {
        const int sizes[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
        
        sample_rate_ = sample_rate;
        
        for(size_t i = 0; i < 8; i++)
        {
            if(i < 4)
            {
                delay_[i].buf = delay_lines_;
                delay_[i].buf += (i > 0) ? sizes[i - 1] : 0;
                delay_[i].buffer_size = sizes[i];
            }
            else
            {
                delay_[i].buf = delay_lines_;
                delay_[i].buf
                    += sizes[0] + sizes[1] + sizes[2] + sizes[3] + (i > 4 ? sizes[i - 1] : 0);
                delay_[i].buffer_size = sizes[i];
            }
            delay_[i].write_pos = 0;
            delay_[i].filter_state = 0.f;
            delay_[i].read_pos = 0;
            delay_[i].read_pos_frac = 0;
            delay_[i].read_pos_frac_inc = 0;
        }
        
        for(size_t i = 0; i < DSY_SMALLREVERB_MAX_SIZE; i++)
        {
            delay_lines_[i] = 0.f;
        }
        
        feedback_ = 0.97f;
        lpfreq_ = 10000.f;
        dry_wet_ = 0.5f; // Initial dry/wet value (50% dry, 50% wet)
        
        SetFeedback(feedback_);
        SetLpFreq(lpfreq_);
    }

    /** Process a block of audio through the reverb module
        \param in1 Left input signal
        \param in2 Right input signal
        \param out1 Left output signal
        \param out2 Right output signal
    */
    void Process(const float&  in1,
                 const float&  in2,
                 float* out1,
                 float* out2)
    {
        float leftout, rightout;
        float input;
        
        input = (in1 + in2) * 0.5f;
        
        leftout = delay_[0].filter_state + delay_[2].filter_state + delay_[4].filter_state + delay_[6].filter_state;
        rightout = delay_[1].filter_state + delay_[3].filter_state + delay_[5].filter_state + delay_[7].filter_state;
        
        // Apply dry/wet mix
        *out1 = (1.0f - dry_wet_) * in1 + dry_wet_ * leftout;
        *out2 = (1.0f - dry_wet_) * in2 + dry_wet_ * rightout;
        
        // Update delays
        for(size_t i = 0; i < 8; i++)
        {
            int write_pos = delay_[i].write_pos;
            float val;
            
            if(i & 1)
            {
                val = input + feedback_ * delay_[i ^ 1].filter_state;
            }
            else
            {
                val = input + feedback_ * delay_[i ^ 1].filter_state;
            }
            
            // Low pass filter
            val = val * (1.0f - lpcoef_) + lpcoef_ * delay_[i].filter_state;
            delay_[i].filter_state = val;
            
            // Write to buffer
            delay_[i].buf[write_pos] = val;
            
            // Increment write position
            write_pos = (write_pos + 1) % delay_[i].buffer_size;
            delay_[i].write_pos = write_pos;
        }
    }

    /** Set feedback amount
        \param feedback Amount of feedback in the reverb
    */
    void SetFeedback(float feedback)
    {
        feedback_ = feedback;
    }

    /** Set cutoff frequency of internal lowpass filter
        \param freq Frequency in Hz
    */
    void SetLpFreq(float freq)
    {
        lpfreq_ = freq;
        lpcoef_ = 1.f - (2.f * PI * lpfreq_ / sample_rate_);
        lpcoef_ = lpcoef_ > 0.f ? lpcoef_ : 0.f;
        lpcoef_ = lpcoef_ < 1.f ? lpcoef_ : 0.99999f;
    }

    /** Set dry/wet mix
        \param amount Amount of wet signal (0.0 = 100% dry, 1.0 = 100% wet)
    */
    void SetDryWet(float amount)
    {
        dry_wet_ = amount;
        // Clamp value to range [0.0, 1.0]
        dry_wet_ = dry_wet_ < 0.0f ? 0.0f : dry_wet_;
        dry_wet_ = dry_wet_ > 1.0f ? 1.0f : dry_wet_;
    }

    /** Get current dry/wet mix
        \return Current dry/wet value
    */
    float GetDryWet()
    {
        return dry_wet_;
    }

  private:
    SmallReverbDl delay_[8];
    float         delay_lines_[DSY_SMALLREVERB_MAX_SIZE];
    float         feedback_, lpfreq_, lpcoef_, sample_rate_;
    float         dry_wet_; // Dry/wet ratio (0.0 = 100% dry, 1.0 = 100% wet)
};
} // namespace daisysp

#endif 