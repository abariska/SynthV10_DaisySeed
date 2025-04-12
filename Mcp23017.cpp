#include "Mcp23017.h"
#include "daisy_seed.h"
#include <unordered_map>

using namespace daisy;
extern DaisySeed hw;

Mcp23017 mcp_1; 
Mcp23017 mcp_2; 

void InitMcp1() {
    Mcp23017::Config mcp_1_Config;
    mcp_1_Config.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    mcp_1_Config.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_1MHZ;
    mcp_1_Config.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    mcp_1_Config.transport_config.i2c_config.pin_config.scl = seed::D13;
    mcp_1_Config.transport_config.i2c_config.pin_config.sda = seed::D14;
    mcp_1_Config.transport_config.i2c_address = 0x21;
    mcp_1.Init(mcp_1_Config);
}
void SetMcp_1_Ports() {
    mcp_1.PinMode(0, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(1, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(2, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(3, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(4, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(5, MCPMode::INPUT_PULLUP, 1); 
    mcp_1.PinMode(6, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(7, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(8, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(9, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(10, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(11, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(12, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(13, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(14, MCPMode::INPUT_PULLUP, 1);
    mcp_1.PinMode(15, MCPMode::INPUT_PULLUP, 1);
}

void InitMcp2() {
    Mcp23017::Config mcp_2_Config;
    mcp_2_Config.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    mcp_2_Config.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_1MHZ;
    mcp_2_Config.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;   
    mcp_2_Config.transport_config.i2c_config.pin_config.scl = seed::D13;
    mcp_2_Config.transport_config.i2c_config.pin_config.sda = seed::D14;
    mcp_2_Config.transport_config.i2c_address = 0x20;
    mcp_2.Init(mcp_2_Config);
}    

void SetMcp_2_Ports() {
    mcp_2.PinMode(0, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(1, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(2, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(3, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(4, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(5, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(6, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(7, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(8, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(9, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(10, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(11, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(12, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(13, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(14, MCPMode::INPUT_PULLUP, 1);
    mcp_2.PinMode(15, MCPMode::INPUT_PULLUP, 1);
}

void InitMcp() {
    InitMcp1();
    SetMcp_1_Ports();
    InitMcp2();
    SetMcp_2_Ports();
}
void LedToggle (bool state, uint8_t output_pin, bool invert) {
    bool current_state = state ? 1 : 0;
    mcp_1.WritePin(output_pin, current_state);
}
