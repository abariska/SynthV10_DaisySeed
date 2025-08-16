#pragma once
#include "stm32h7xx_hal.h"
#include <stdint.h>

#define LOG_BUF_SIZE 512

void log_init(UART_HandleTypeDef* huart);
void log_write(const char* str);
void log_flush();