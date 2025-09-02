#include "sx1509_expander.h"
#include "parameters.h"

SX1509 sx1509_buttons;
SX1509 sx1509_encoders;
SX1509 sx1509_leds;

void InitSX1509Buttons() {

    SX1509::Config i2c_conf_buttons;
    i2c_conf_buttons.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_buttons.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_buttons.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_buttons.transport_config.i2c_config.pin_config.scl = seed::D11;
    i2c_conf_buttons.transport_config.i2c_config.pin_config.sda = seed::D12;
    i2c_conf_buttons.transport_config.i2c_address = 0x3E;
    sx1509_buttons.Init(i2c_conf_buttons);
    sx1509_buttons.Check();
}

void InitSX1509Encoders() {
    
    SX1509::Config i2c_conf_encoders;
    i2c_conf_encoders.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_encoders.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_encoders.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_encoders.transport_config.i2c_config.pin_config.scl = seed::D11;
    i2c_conf_encoders.transport_config.i2c_config.pin_config.sda = seed::D12;
    i2c_conf_encoders.transport_config.i2c_address = 0x3F;
    sx1509_encoders.Init(i2c_conf_encoders);
    sx1509_encoders.Check();    
}

void InitSX1509Leds() {

    SX1509::Config i2c_conf_leds;
    i2c_conf_leds.transport_config.i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_leds.transport_config.i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_leds.transport_config.i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_leds.transport_config.i2c_config.pin_config.scl = seed::D11;
    i2c_conf_leds.transport_config.i2c_config.pin_config.sda = seed::D12;
    i2c_conf_leds.transport_config.i2c_address = 0x70;
    sx1509_leds.Init(i2c_conf_leds);
    sx1509_leds.Check();
}

void InitSX1509Extenders() {

    InitSX1509Buttons();
    InitSX1509Encoders();
    InitSX1509Leds();

    for (int i = 0; i < 16; i++) {
        sx1509_buttons.SetPinMode(i, SX_PIN_INPUT_PULLUP, 1);
        sx1509_buttons.DebouncePin(i);
    }
    sx1509_buttons.DebounceConfig(3);
    sx1509_buttons.ReadAllPins();

    for (int i = 0; i < 16; i++) {
        sx1509_encoders.SetPinMode(i, SX_PIN_INPUT_PULLUP, 1);
    }
    sx1509_encoders.ReadAllPins();

    for (int i = 0; i < 16; i++) {
        sx1509_leds.SetPinMode(i, SX_PIN_OUTPUT, 0);
    }

    sx1509_leds.WritePin(LED_OSC_1, paramManager.GetBool(OSC_ACTIVE[0]));
    sx1509_leds.WritePin(LED_OSC_2, paramManager.GetBool(OSC_ACTIVE[1]));
    sx1509_leds.WritePin(LED_OSC_3, paramManager.GetBool(OSC_ACTIVE[2]));
    // sx1509_leds.WritePin(LED_LFO, params.lfo.active);
    // sx1509_leds.WritePin(LED_MTX, params.mtx.active);

    System::Delay(10);
}

int8_t EncoderInc(uint8_t pin_a, uint8_t pin_b) {

    static uint8_t a_[16] = {0};
    static uint8_t b_[16] = {0};
    static uint32_t last_increment_time_[16] = {0};
    int8_t inc_ = 0;

    // Shift Button states to debounce
    a_[pin_a] = (a_[pin_a] << 1) | sx1509_encoders.CurrentPinState(pin_a);
    b_[pin_b] = (b_[pin_b] << 1) | sx1509_encoders.CurrentPinState(pin_b);

    // infer increment direction
    if((a_[pin_a] & 0x03) == 0x02 && (b_[pin_b] & 0x03) == 0x00)
    {
        inc_ = 1;
    }
    else if((b_[pin_b] & 0x03) == 0x02 && (a_[pin_a] & 0x03) == 0x00)
    {
        inc_ = -1;
    }
	if (inc_ != 0) {
		// Determine rotation speed
        uint32_t now = System::GetNow();
		uint32_t time_diff = now - last_increment_time_[pin_a];
		
		int8_t speed_factor_;  
		// Update speed multiplier
		if (time_diff < 10) {  // Fast rotation
            if (time_diff < 5) {
                speed_factor_ = 100;
            }
            else {
                speed_factor_ = 10;
            }
		} else {  // Slow rotation
			speed_factor_ = 1;
		}
		// Update last change time
		last_increment_time_[pin_a] = now;
		
		// Apply speed multiplier
		inc_ *=  speed_factor_;
	}
    return inc_;
}