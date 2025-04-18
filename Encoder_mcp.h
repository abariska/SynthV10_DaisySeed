#ifndef ENCODER_MCP_H
#define ENCODER_MCP_H

#include "Mcp23017.h"
#include "Button.h"

extern DaisySeed hw;
extern int encoderIncs[4];

class Enc_mcp
{
  private:
    Mcp23017& mcp;
    uint32_t last_update_;
    uint32_t last_increment_time_;
    bool     updated_;
    Button_mcp   sw_;
    uint8_t  a_, b_;
    int32_t  inc_;
    uint8_t  pin_a_, pin_b_, pin_sw_;  

  public:
    Enc_mcp(Mcp23017& mcp, uint8_t pin_a, uint8_t pin_b, uint8_t pin_sw, bool inverted = false) 
        : mcp(mcp), 
        last_update_(0),
        updated_(false),
        sw_(mcp, pin_sw, inverted),
        pin_a_(pin_a),
        pin_b_(pin_b),
        pin_sw_(pin_sw)
    {
        // Set initial states
        inc_ = 0;
        a_ = b_ = 0xff;
        
        // Initialize button
        sw_.Init();
    }
    ~Enc_mcp() {}

    /** Called at update_rate to debounce and handle timing for the switch.
     * In order for events not to be missed, its important that the Edge/Pressed checks be made at the same rate as the debounce function is being called.
     */
    void Debounce();

    /** Returns +1 if the encoder was turned clockwise, -1 if it was turned counter-clockwise, or 0 if it was not just turned. */
    inline int32_t Increment() const { return updated_ ? inc_ : 0; }

    /** Returns true if the encoder was just pressed. */
    inline bool RisingEdge() const { return sw_.RisingEdge(); }

    /** Returns true if the encoder was just released. */
    inline bool FallingEdge() const { return sw_.FallingEdge(); }

    /** Returns true while the encoder is held down.*/
    inline bool Pressed() const { return sw_.IsPressed(); }

    /** Returns the time in milliseconds that the encoder has been held down. */
    inline float TimeHeldMs() const { return sw_.TimeHeldMs(); }

};

#endif