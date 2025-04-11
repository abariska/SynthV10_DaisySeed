#include "DisplayOLED.h"

MyOledDisplay display;

void Display_Init() {
    MyOledDisplay::Config disp_cfg;
    disp_cfg.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_1;
    disp_cfg.driver_config.transport_config.spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_8;

    disp_cfg.driver_config.transport_config.spi_config.pin_config.sclk = seed::D8;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.miso = seed::D9;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.mosi = seed::D10;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.nss = seed::D7;
    disp_cfg.driver_config.transport_config.pin_config.dc = seed::D12;
    disp_cfg.driver_config.transport_config.pin_config.reset = seed::D11;

    display.Init(disp_cfg);
}

