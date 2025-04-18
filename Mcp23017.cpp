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
    for (uint8_t i = 0; i < 15; i++) {
            mcp_1.PinMode(i, MCPMode::INPUT_PULLUP, 1);
        }
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
    for (uint8_t i = 0; i < 15; i++) {
        mcp_2.PinMode(i, MCPMode::INPUT_PULLUP, 1);
    }
}    


void InitMcp() {
    InitMcp1();
    InitMcp2();
}
// void LedToggle (bool state, uint8_t output_pin, bool invert) {
//     bool current_state = state ? 1 : 0;
//     mcp_1.WritePin(output_pin, current_state);
// }
