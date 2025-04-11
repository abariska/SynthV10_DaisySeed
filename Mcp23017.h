#ifndef MCP23017_H
#define MCP23017_H

#include "daisy_seed.h"      
#include "dev/mcp23x17.h"    

using namespace daisy;
using Mcp23017 = daisy::Mcp23X17<daisy::Mcp23017Transport>;

// MCP1
#define BUTTON_OSC_1 7
#define BUTTON_OSC_2 6
#define BUTTON_OSC_3 5
#define BUTTON_FLT 4
#define BUTTON_AMP 3
#define BUTTON_LFO 2
#define BUTTON_MTX 1
#define BUTTON_FX 0
#define BUTTON_SHIFT 9
#define BUTTON_BACK 8
#define LED_OSC_1 13
#define LED_OSC_2 14
#define LED_OSC_3 15
#define LED_FX 12
#define LED_MIDI 11
#define LED_OUT 10
// MCP2 
#define ENC_1_SW 7
#define ENC_1_B 6
#define ENC_1_A 5
#define ENC_2_SW 4
#define ENC_2_B 3
#define ENC_2_A 2
#define ENC_3_SW 1
#define ENC_3_B 0
#define ENC_3_A 8
#define ENC_4_SW 9
#define ENC_4_B 10
#define ENC_4_A 11
#define ENC_DIAL_SW 12
#define ENC_DIAL_B 13
#define ENC_DIAL_A 14

extern Mcp23017 mcp_1;
extern Mcp23017 mcp_2;

// Initialization functions
void InitMcp();

#endif