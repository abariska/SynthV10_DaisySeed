#include "Encoder_mcp.h"

extern DaisySeed hw;

void Enc_mcp::Debounce()
{
    // Update no more than 1kHz
    uint32_t now = System::GetNow();
    updated_     = false;

    if(now - last_update_ >= 1)
    {
        last_update_ = now;
        updated_     = true;

        // Shift pin states for debouncing
        a_ = (a_ << 1) | mcp.GetPin(pin_a_);
        b_ = (b_ << 1) | mcp.GetPin(pin_b_);

        // Determine rotation direction
        inc_ = 0; // reset inc_ first
        if((a_ & 0x03) == 0x02 && (b_ & 0x03) == 0x00)
        {
            inc_ = 1;
        }
        else if((b_ & 0x03) == 0x02 && (a_ & 0x03) == 0x00)
        {
            inc_ = -1;
        }

        if (inc_ != 0) {
            // Determine rotation speed
            uint32_t time_diff = now - last_increment_time_;
            
            int    speed_factor_;  
            // Update speed multiplier
            if (time_diff < 150) {  // Fast rotation
                speed_factor_ = 5;
            } else if (time_diff < 70) {  // Very fast rotation
                // speed_factor_ = 10;
                speed_factor_ = 10;
            } else {  // Slow rotation
                speed_factor_ = 1;
            }
            
            // Update last change time
            last_increment_time_ = now;
            
            // Apply speed multiplier
            inc_ *=  speed_factor_;
        }
        // Update button state
        sw_.Update(now);

        // hw.PrintLine("Encoder: %d", test_int + inc_);
    }
}