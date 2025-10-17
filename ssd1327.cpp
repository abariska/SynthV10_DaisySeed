#include "ssd1327.h"
#include "daisy.h"

using namespace daisy;

SpiHandle spi_display;
SpiHandle::Config spi_config;
GPIO pin_dc;
GPIO pin_reset;
GPIO pin_cs;
u8g2_t myDisplay;
/*------------------------------------------------------------------------------------------------------*/

void SPI_Config()
{
    
    // SPI peripheral config
    spi_config.periph = SpiHandle::Config::Peripheral::SPI_1;
    spi_config.mode   = SpiHandle::Config::Mode::MASTER;
    spi_config.direction
        = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
    spi_config.datasize       = 8;
    spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
    spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
    spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;
    // SPI pin config
    spi_config.pin_config.sclk = Pin(PORTG, 11);
    spi_config.pin_config.mosi = Pin(PORTA, 7);
    spi_config.pin_config.nss  = Pin(PORTG, 10);
    spi_display.Init(spi_config);
}

void SPI_Init()
{
    SPI_Config();
    // SSD1327 control pin config
    pin_dc.Init(Pin(PORTC, 1), GPIO::Mode::OUTPUT);
    pin_reset.Init(Pin(PORTB, 1), GPIO::Mode::OUTPUT);
    pin_cs.Init(Pin(PORTG, 10), GPIO::Mode::OUTPUT);
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	  switch(msg)
	  {
	  case U8X8_MSG_DELAY_MILLI:
		  System::Delay(arg_int);
		  break;
	  case U8X8_MSG_GPIO_CS:
		  pin_cs.Write(arg_int);
		  break;
	  case U8X8_MSG_GPIO_DC:
		  pin_dc.Write(arg_int);
		  break;
	  case U8X8_MSG_GPIO_RESET:
		  pin_reset.Write(arg_int);
		  break;
	  }
	  return 1;
}

uint8_t u8x8_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	  switch(msg)
	  {
	  case U8X8_MSG_BYTE_SET_DC:
		  pin_dc.Write(arg_int);
		  break;
	  case U8X8_MSG_BYTE_SEND:
		  spi_display.DmaTransmit((uint8_t *)arg_ptr, arg_int, NULL, NULL, NULL);
		  break;
	  case U8X8_MSG_BYTE_START_TRANSFER:
		  pin_cs.Write(0);
		  break;
	  case U8X8_MSG_BYTE_END_TRANSFER:
		  pin_cs.Write(1);
		  break;
	  }
	  return 1;
}

void ssd1327_Init()
{
    SPI_Init();
    u8g2_Setup_ssd1327_ws_128x128_1(&myDisplay, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&myDisplay);
    u8g2_SetPowerSave(&myDisplay, 0);
}

void ssd1327_Display()
{
	u8g2_ClearDisplay(&myDisplay);
	u8g2_SetFont(&myDisplay, u8g2_font_ncenB14_tr);
	u8g2_DrawStr(&myDisplay, 0,15,"Hello world");
	u8g2_DrawCircle(&myDisplay, 60, 30, 10, U8G2_DRAW_ALL);
	u8g2_SendBuffer(&myDisplay);
}