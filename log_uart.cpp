#include "log_uart.h"
#include <string.h>

static UART_HandleTypeDef* uart;
static char logBuffer[LOG_BUF_SIZE];
static volatile uint16_t logWritePos = 0;
static volatile uint16_t logReadPos = 0;
static volatile uint8_t dmaBusy = 0;

void log_init(UART_HandleTypeDef* huart_ptr) {
    uart = huart_ptr;
}

void log_write(const char* str) {
    while (*str) {
        uint16_t next = (logWritePos + 1) % LOG_BUF_SIZE;
        if (next != logReadPos) { // перевірка переповнення
            logBuffer[logWritePos] = *str++;
            logWritePos = next;
        } else {
            break; // буфер повний, пропускаємо символи
        }
    }
}

static void log_start_dma() {
    if (dmaBusy) return;
    if (logReadPos == logWritePos) return;

    uint16_t len = (logWritePos > logReadPos) ? 
                   (logWritePos - logReadPos) : 
                   (LOG_BUF_SIZE - logReadPos);

    dmaBusy = 1;
    HAL_UART_Transmit_DMA(uart, (uint8_t*)&logBuffer[logReadPos], len);
}

void log_flush() {
    log_start_dma();
}

// Викликати в DMA Complete callback:
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == uart) {
        logReadPos = (logReadPos + (logWritePos > logReadPos ? (logWritePos - logReadPos) : (LOG_BUF_SIZE - logReadPos))) % LOG_BUF_SIZE;
        dmaBusy = 0;
        log_start_dma(); // перевіряємо чи є ще дані
    }
}