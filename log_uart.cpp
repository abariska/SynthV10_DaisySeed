#include <string.h>
#include "daisy.h"
#include "daisy_core.h"
#include "log_uart.h"

using namespace daisy;

UartHandler uart_serial;

void UartSerialInit() {
    UartHandler::Config cfg;
    cfg.periph         = UartHandler::Config::Peripheral::LPUART_1; // USART1
    cfg.pin_config.tx  = Pin(PORTB, 6); // D13 -> USART1_TX
    cfg.pin_config.rx  = Pin(PORTB, 7); // D14 -> USART1_RX
    cfg.baudrate       = 115200;
    cfg.wordlength     = UartHandler::Config::WordLength::BITS_8;
    cfg.stopbits       = UartHandler::Config::StopBits::BITS_1;
    cfg.parity         = UartHandler::Config::Parity::NONE;
    cfg.mode           = UartHandler::Config::Mode::TX_RX;
    uart_serial.Init(cfg);
    

}

void UartPrint(const char* text) {
    char test[40];
    sprintf(test, "%s\r\n", text);
    uart_serial.BlockingTransmit((uint8_t*)test, strlen(test), 1000);
}

void UartPrint(int num) {
    char test[40];
    sprintf(test, "%d\r\n", num);
    uart_serial.BlockingTransmit((uint8_t*)test, strlen(test), 1000);
}

void UartPrint(const char* text, int num) {
    char test[40];
    sprintf(test, "%s %d\r\n", text, num);
    uart_serial.BlockingTransmit((uint8_t*)test, strlen(test), 1000);
}
