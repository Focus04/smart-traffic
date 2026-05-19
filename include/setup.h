#pragma once

void SystemClock_Config();
void GPIO_Init();
void UART2_Init();
void I2C_Init();

void SysTick_Handler();

int _write(int file, char *ptr, int len);