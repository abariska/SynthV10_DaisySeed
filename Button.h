#ifndef BUTTON_H
#define BUTTON_H

#include "Mcp23017.h"

class Button_mcp {
private:
    Mcp23017& mcp;        // Reference to MCP23017 controller
    int pin;              // Pin number on MCP23017
    bool inverted;        // Logic inversion flag
    
    uint32_t last_update_;
    bool updated_;
    uint8_t state_;
    float rising_edge_time_;
    
public:
    // Constructor with MCP23017 pin assignment and inversion capability
    Button_mcp(Mcp23017& mcp_ref, int pinValue, bool invert = false);
    
    // Button initialization
    void Init();
    
    // Update button parameters
    void UpdateParams(int pinValue, bool invert);
    
    // Button debouncing method (separate, as in libDaisy)
    void Debounce();
    
    // Update button state - calls Debounce() for compatibility
    void Update(unsigned long currentTime);
    
    // Check if button was just pressed (rising edge)
    bool RisingEdge() const;
    
    // Check if button was just released (falling edge)
    bool FallingEdge() const;
    
    // Check if button is pressed
    bool IsPressed() const;
    
    // Check if button was just pressed (for compatibility)
    bool WasPressed() const;
    
    // Check if button was just released (for compatibility)
    bool WasReleased() const;
    
    // Check if button is long pressed
    bool IsLongPressed() const;
    
    // Get button press time in milliseconds
    float TimeHeldMs() const;
    
    // Get current raw button state
    bool RawState();
    
    // Get current state_ value for diagnostics
    uint8_t GetState() const { return state_; }
};

#endif // BUTTON_H 