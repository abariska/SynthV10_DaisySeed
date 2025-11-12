
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "per/gpio.h"
#include "per/spi.h"
#include "daisy.h"

// Display controller types
#define SSD1327 1327
#define SSD1363 1363

#define DISPLAY_CONTROLLER SSD1327 // SSD1327 or SSD1363

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

#define SPI_CLK_PIN PORTG, 11 //D8
#define SPI_NSS_PIN PORTG, 10 //D7
#define SPI_MOSI_PIN PORTA, 7 //D18
#define SPI_DC_PIN PORTC, 1 //D20
#define SPI_RESET_PIN PORTB, 1 //D17

/****************************************************************************************
    //Use the  library function definition
*****************************************************************************************/
//GPIO config




#endif
