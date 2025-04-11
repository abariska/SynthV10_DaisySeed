#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#include "daisy_seed.h"  // For basic Daisy functions
#include "dev/oled_ssd130x.h"  // For OLED display operations

using namespace daisy;

// Display type
using MyOledDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;

// Global display object
extern MyOledDisplay display;

// Display functions
void Display_Init();

#endif