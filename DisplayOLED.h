#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#include "daisy_seed.h"  // Для базових функцій Daisy
#include "dev/oled_ssd130x.h"  // Для роботи з OLED дисплеєм

using namespace daisy;

// Тип дисплея
using MyOledDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;

// Глобальний об'єкт дисплея
extern MyOledDisplay display;

// Функції для роботи з дисплеєм
void Display_Init();

#endif