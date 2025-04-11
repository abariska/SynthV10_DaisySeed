#include "Button.h"

// Using global variable hw from daisy_seed.h
// using namespace daisy;
// extern DaisySeed hw;

// Constructor with inversion capability
Button_mcp::Button_mcp(Mcp23017& mcp_ref, int pinValue, bool invert) 
    : mcp(mcp_ref), pin(pinValue), inverted(invert) {
    state_ = 0;
    updated_ = false;
    last_update_ = 0;
    rising_edge_time_ = 0.0f;
}

// Button initialization
void Button_mcp::Init() {
    // Initialize button
    state_ = 0;
    updated_ = false;
    last_update_ = 0;
    rising_edge_time_ = 0.0f;
}

// Button debouncing method (separate, as in libDaisy)
void Button_mcp::Debounce() {
    bool new_val = mcp.ReadPin(pin);
    
    // Invert value if needed
    if (inverted) {
        new_val = !new_val;
    }
    
    // Shift state left and add new bit
    state_ = (state_ << 1) | (new_val ? 1 : 0);
}

// Update button state - calls Debounce() for compatibility
void Button_mcp::Update(unsigned long currentTime) {
    if (currentTime - last_update_ >= 1) { // 1ms interval
        Debounce();
        last_update_ = currentTime;
        updated_ = true;
    }
}

// Check if button was just pressed (rising edge)
bool Button_mcp::RisingEdge() const {
    return (state_ & 0x03) == 0x02; // Check only last 2 bits
}

// Check if button was just released (falling edge)
bool Button_mcp::FallingEdge() const {
    return (state_ & 0x03) == 0x01; // Check only last 2 bits
}

// Check if button is pressed
bool Button_mcp::IsPressed() const {
    return (state_ & 0x03) == 0x03; // Check only last 2 bits
}

// Check if button was just pressed
bool Button_mcp::WasPressed() const {
    return RisingEdge();
}

// Check if button was just released
bool Button_mcp::WasReleased() const {
    return FallingEdge();
}

// Check if button is long pressed
bool Button_mcp::IsLongPressed() const {
    return IsPressed() && TimeHeldMs() > 1000.0f;
}

// Get button press time in milliseconds
float Button_mcp::TimeHeldMs() const {
    return rising_edge_time_;
}

// Get current raw button state
bool Button_mcp::RawState() {
    bool raw = mcp.ReadPin(pin);
    return inverted ? !raw : raw;
}

void Button_mcp::UpdateParams(int pinValue, bool invert) {
    pin = pinValue;
    inverted = invert;
    Init();  // Reinitialize button with new parameters
}


