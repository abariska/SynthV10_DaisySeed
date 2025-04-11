#ifndef LED_H
#define LED_H

#include "Mcp23017.h"

class Led_mcp {
private:
    Mcp23017& mcp;       // Reference to MCP23017 controller
    int pin;             // Pin number on MCP23017
    bool state;          // Current LED state (on/off)
    bool inverted;       // LED logic inversion
    
public:
    // Constructor with MCP23017 pin assignment
    Led_mcp(Mcp23017& mcp, int pin, bool inverted = false)
        : mcp(mcp), pin(pin), state(false), inverted(inverted) {}
    
    // Turn LED on
    void On() {
        mcp.WritePin(pin, inverted ? 0 : 1);
        state = true;
    }
    
    // Turn LED off
    void Off() {
        mcp.WritePin(pin, inverted ? 1 : 0);
        state = false;
    }
    
    // Toggle LED state
    void Toggle() {
        state = !state;
        mcp.WritePin(pin, inverted ? !state : state);
    }
    
    // Set LED state
    void SetState(bool newState) {
        state = newState;
        mcp.WritePin(pin, inverted ? !state : state);
    }
    
    // Get current LED state
    bool GetState() const {
        return state;
    }
};


#endif // LED_H 