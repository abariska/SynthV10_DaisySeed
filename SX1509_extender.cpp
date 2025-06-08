#include "SX1509_extender.h"

SX1509 sx1509_buttons;
SX1509 sx1509_encoders;
SX1509 sx1509_leds;
    

void InitSX1509Buttons() {

    SX1509::Config i2c_conf_buttons;
    i2c_conf_buttons.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_buttons.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_buttons.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_buttons.transport_config.i2c_config.pin_config.scl = seed::D13;
    i2c_conf_buttons.transport_config.i2c_config.pin_config.sda = seed::D14;
    i2c_conf_buttons.transport_config.i2c_address = 0x3E;
    sx1509_buttons.Init(i2c_conf_buttons);
    sx1509_buttons.Check();
    sx1509_buttons.DebounceTime(1);

    for (int i = 0; i < 16; i++) {
        sx1509_buttons.SetPinMode(i, 1, 1);
    }
}

void InitSX1509Encoders() {
    
    SX1509::Config i2c_conf_encoders;
    i2c_conf_encoders.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_encoders.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_encoders.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_encoders.transport_config.i2c_config.pin_config.scl = seed::D13;
    i2c_conf_encoders.transport_config.i2c_config.pin_config.sda = seed::D14;
    i2c_conf_encoders.transport_config.i2c_address = 0x3F;
    sx1509_encoders.Init(i2c_conf_encoders);
    sx1509_encoders.Check();
    sx1509_encoders.DebounceTime(1);

    for (int i = 0; i < 16; i++) {
        sx1509_encoders.SetPinMode(i, 1, 1);
    }
    
}

void InitSX1509Leds() {

    SX1509::Config i2c_conf_leds;
    i2c_conf_leds.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_leds.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_leds.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_leds.transport_config.i2c_config.pin_config.scl = seed::D13;
    i2c_conf_leds.transport_config.i2c_config.pin_config.sda = seed::D14;
    i2c_conf_leds.transport_config.i2c_address = 0x70;
    sx1509_leds.Init(i2c_conf_leds);
    sx1509_leds.Check();

    for (int i = 0; i < 16; i++) {
        sx1509_leds.SetPinMode(i, 0, 1);
    }
}

void InitSX1509Extenders() {
    
    InitSX1509Buttons();
    InitSX1509Encoders();
    InitSX1509Leds();
}


