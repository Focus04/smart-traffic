#pragma once

void SystemClock_Config();
void GPIO_Init();
void UART2_Init();
void I2C_Init();
void LCD_Init();
void LCD_PrintLine(uint8_t row, const char *str);

void SysTick_Handler();

int _write(int file, char *ptr, int len);