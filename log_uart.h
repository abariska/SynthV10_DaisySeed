#pragma once

#define LOG_BUF_SIZE 512

void UartSerialInit();
void UartPrint(const char *text);
void UartPrint(int num);
void UartPrint(const char *text, int num);
void UartPrintf(const char *text, float num);
