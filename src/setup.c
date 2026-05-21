#include "defines.h"

#include "stm32l4xx_hal.h"

UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

volatile uint32_t EW_traffic_cnt = 0;
volatile uint32_t NS_traffic_cnt = 0;

void SystemClock_Config()
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    while (1) {}

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    while (1) {}
}

void GPIO_Init()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = EW_GREEN | EW_YELLOW | EW_RED | NS_GREEN | NS_YELLOW | NS_RED;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = E_SENSOR_1 | E_SENSOR_2 | W_SENSOR_1 | W_SENSOR_2 | N_SENSOR_1 | N_SENSOR_2 | S_SENSOR_1 | S_SENSOR_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void UART2_Init()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = UART_RX | UART_TX;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
}

void I2C_Init()
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = I2C_SCL | I2C_SDA;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
}

void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(E_SENSOR_1);
  HAL_GPIO_EXTI_IRQHandler(E_SENSOR_2);
  HAL_GPIO_EXTI_IRQHandler(W_SENSOR_1);
}

void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(W_SENSOR_2);
  HAL_GPIO_EXTI_IRQHandler(N_SENSOR_1);
  HAL_GPIO_EXTI_IRQHandler(N_SENSOR_2);
  HAL_GPIO_EXTI_IRQHandler(S_SENSOR_1);
  HAL_GPIO_EXTI_IRQHandler(S_SENSOR_2);
}

void SysTick_Handler()
{
  HAL_IncTick();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t current_time = HAL_GetTick();
  static uint32_t last_ew_tick = 0;
  static uint32_t last_ns_tick = 0;

  if (GPIO_Pin == E_SENSOR_1 || GPIO_Pin == E_SENSOR_2 || GPIO_Pin == W_SENSOR_1 || GPIO_Pin == W_SENSOR_2)
  {
    if (current_time - last_ew_tick > SENSOR_DEBOUNCE_MS)
    {
      EW_traffic_cnt++;
      last_ew_tick = current_time;
    }
  }
  else if (GPIO_Pin == N_SENSOR_1 || GPIO_Pin == N_SENSOR_2 || GPIO_Pin == S_SENSOR_1 || GPIO_Pin == S_SENSOR_2)
  {
    if (current_time - last_ns_tick > SENSOR_DEBOUNCE_MS)
    {
      NS_traffic_cnt++;
      last_ns_tick = current_time;
    }
  }
}

int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

void LCD_WriteNibble(uint8_t nibble, uint8_t rs)
{
  uint8_t data;
  uint8_t backlight = 0x08;

  data = (nibble & 0xF0) | backlight | rs | 0x04;
  HAL_I2C_Master_Transmit(&hi2c1, 0x4E, &data, 1, 100);

  data &= ~0x04;
  HAL_I2C_Master_Transmit(&hi2c1, 0x4E, &data, 1, 100);

  HAL_Delay(1);
}

void LCD_SendByte(uint8_t byte, uint8_t rs)
{
  LCD_WriteNibble(byte & 0xF0, rs);
  LCD_WriteNibble((byte << 4) & 0xF0, rs);
}

void LCD_Init()
{
  HAL_Delay(50);

  LCD_WriteNibble(0x30, 0);
  HAL_Delay(5);
  LCD_WriteNibble(0x30, 0);
  HAL_Delay(1);
  LCD_WriteNibble(0x30, 0);
  HAL_Delay(1);

  LCD_WriteNibble(0x20, 0);
  HAL_Delay(5);

  LCD_SendByte(0x28, 0);
  HAL_Delay(1);
  LCD_SendByte(0x0C, 0);
  HAL_Delay(1);
  LCD_SendByte(0x01, 0);
  HAL_Delay(2);
  LCD_SendByte(0x06, 0);
  HAL_Delay(1);
}

void LCD_PrintLine(uint8_t row, const char *str)
{
  uint8_t addr = (row == 0) ? 0x00 : (row == 1) ? 0x40 : (row == 2) ? 0x14 : 0x54;

  LCD_SendByte(0x80 | addr, 0);

  int len = 0;
  while (str[len] != '\0' && len < 20)
  {
    LCD_SendByte(str[len], 1);
    len++;
  }
  while (len < 20)
  {
    LCD_SendByte(' ', 1);
    len++;
  }
}