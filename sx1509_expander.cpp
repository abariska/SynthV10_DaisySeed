#include <stdint.h>
#include "sx1509_expander.h"
#include "daisy_core.h"
#include "parameters.h"
#include "menu.h"
#include "log_uart.h"

using namespace std;

SX1509 sx1509_buttons;
SX1509 sx1509_encoders;
SX1509 sx1509_leds;
using M = ModSource;


void InitSX1509Buttons()
{

    SX1509::Config i2c_conf_buttons;
    i2c_conf_buttons.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_buttons.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_buttons.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_buttons.transport_config.i2c_config.pin_config.scl = Pin(PORTB, 8);
    i2c_conf_buttons.transport_config.i2c_config.pin_config.sda = Pin(PORTB, 9);
    i2c_conf_buttons.transport_config.i2c_address = 0x3E;
    sx1509_buttons.Init(i2c_conf_buttons);
    sx1509_buttons.Check();
}

void InitSX1509Encoders()
{

    SX1509::Config i2c_conf_encoders;
    i2c_conf_encoders.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_encoders.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_encoders.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_encoders.transport_config.i2c_config.pin_config.scl = Pin(PORTB, 8);
    i2c_conf_encoders.transport_config.i2c_config.pin_config.sda = Pin(PORTB, 9);
    i2c_conf_encoders.transport_config.i2c_address = 0x3F;
    sx1509_encoders.Init(i2c_conf_encoders);
    sx1509_encoders.Check();
}

void InitSX1509Leds()
{

    SX1509::Config i2c_conf_leds;
    i2c_conf_leds.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf_leds.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf_leds.transport_config.i2c_config.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf_leds.transport_config.i2c_config.pin_config.scl = Pin(PORTB, 8);
    i2c_conf_leds.transport_config.i2c_config.pin_config.sda = Pin(PORTB, 9);
    i2c_conf_leds.transport_config.i2c_address = 0x70;
    sx1509_leds.Init(i2c_conf_leds);
    sx1509_leds.Check();
}

void UpdateLeds()
{
    sx1509_leds.WritePin(LED_OSC_1, paramManager.GetBool(P::OSC_ACTIVE_1));
    sx1509_leds.WritePin(LED_OSC_2, paramManager.GetBool(P::OSC_ACTIVE_2));
    sx1509_leds.WritePin(LED_OSC_3, paramManager.GetBool(P::OSC_ACTIVE_3)); 
    sx1509_leds.WritePin(LED_STORE, isStoreMode);    

}
void UpdatePWMLeds()
{
    sx1509_leds.WritePWM(LED_LFO, (int)(modulators[static_cast<int>(M::LFO)].value * 255));
}

void UpdateStoreLed()
{
    if (isStoreMode)
    {
        sx1509_leds.WritePin(LED_STORE, updateStoreLed);
        updateStoreLed = !updateStoreLed;
    }
    
}

void InitSX1509Extenders()
{
    InitSX1509Buttons();
    InitSX1509Encoders();
    InitSX1509Leds();

    for (int i = 0; i < 16; i++)
    {
        sx1509_buttons.SetPinMode(i, SX_PIN_INPUT_PULLUP, 1);
        sx1509_buttons.DebouncePin(i);
    }
    sx1509_buttons.DebounceConfig(3);
    sx1509_buttons.ReadAllPins();

    for (int i = 0; i < 16; i++)
    {
        sx1509_encoders.SetPinMode(i, SX_PIN_INPUT_PULLUP, 1);
    }
    sx1509_encoders.ReadAllPins();

    for (int i = 0; i < 16; i++)
    {
        sx1509_leds.SetPinMode(i, SX_PIN_OUTPUT, 0);
    }
    
    for (size_t i = 0; i < 8; i++)
    {
        sx1509_leds.WritePin(i, 1);
        sx1509_leds.WritePin(i - 1, 0);
        System::Delay(100);
    }
    for (size_t i = 6; i > 0; i--)
    {
        sx1509_leds.WritePin(i, 1);
        sx1509_leds.WritePin(i + 1, 0);
        System::Delay(100);
    }
    sx1509_leds.LedDriverInit(LED_LFO, 1, true);
    sx1509_leds.WritePWM(LED_LFO, 0);
    UpdateLeds();

    System::Delay(10);
}

int8_t EncoderInc(uint8_t pin_a, uint8_t pin_b)
{

    static uint8_t a_[16] = {0};
    static uint8_t b_[16] = {0};
    static uint32_t last_increment_time_[16] = {0};

    // Shift Button states to debounce
    a_[pin_a] = (a_[pin_a] << 1) | sx1509_encoders.CurrentPinState(pin_a);
    b_[pin_b] = (b_[pin_b] << 1) | sx1509_encoders.CurrentPinState(pin_b);

    static const int8_t kDeltaLut[16] = {
        // idx = (prev<<2)|curr :  00->00 00->01 00->10 00->11  01->00 01->01 01->10 01->11 ...
             0,   -1,   +1,    0,
            +1,    0,    0,   -1,
            -1,    0,    0,   +1,
             0,   +1,   -1,    0
        };

        uint8_t prev = ((a_[pin_a] >> 1) & 1) | (((b_[pin_b] >> 1) & 1) << 1);
        uint8_t curr =  (a_[pin_a]        & 1) | (( b_[pin_b]        & 1) << 1);
        uint8_t idx  = (prev << 2) | curr;

        static int8_t acc_[16] = {0};
        const uint8_t DETENT = 0;  

        int8_t step = kDeltaLut[idx];
        int8_t inc_ = 0;

        if (step) {
            int8_t s = acc_[pin_a] + step;
            if (s > 3)  s = 3;
            if (s < -3) s = -3;
            acc_[pin_a] = s;
        }
        
        if (curr == DETENT) {
            if (acc_[pin_a] >= +2) inc_ = +1;
            else if (acc_[pin_a] <= -2) inc_ = -1;
            acc_[pin_a] = 0;
        }
        
    if (inc_ != 0)
    {
        // Determine rotation speed
        uint32_t now = System::GetNow();
        uint32_t time_diff = now - last_increment_time_[pin_a];

        int8_t speed_factor_;
        // Update speed multiplier
        if (time_diff < 10)
        { // Fast rotation
            if (time_diff < 6)
            {
                speed_factor_ = 100;
            }
            else
            {
                speed_factor_ = 10;
            }
        }
        else
        { // Slow rotation
            speed_factor_ = 1;
        }
        // Update last change time
        last_increment_time_[pin_a] = now;

        // Apply speed multiplier
        inc_ *= speed_factor_;
    }
    return inc_;
}