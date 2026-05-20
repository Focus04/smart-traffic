#include "defines.h"
#include "setup.h"

#include <stdio.h>

extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;

extern volatile uint32_t EW_traffic_cnt;
extern volatile uint32_t NS_traffic_cnt;

uint32_t green_duration_ew = BASE_GREEN_DURATION_MS;
uint32_t green_duration_ns = BASE_GREEN_DURATION_MS;
uint8_t consecutive_ew_high_traffic = 0;
uint8_t consecutive_ns_high_traffic = 0;
traffic_tier_t current_tier = TIER_0_BASE;

void Set_Traffic_Lights(GPIO_TypeDef *GPIOx, uint16_t lights)
{
  if ((EW_GREEN & lights) && (NS_GREEN & lights))
  {
    printf("FATAL ERROR! Colliding directions cannot be green simultaneously. Failure mode is active.\n");
    HAL_GPIO_WritePin(GPIOx, EW_GREEN | EW_YELLOW | EW_RED | NS_GREEN | NS_YELLOW | NS_RED, GPIO_PIN_RESET);
    while (1)
    {
      HAL_GPIO_TogglePin(GPIOx, EW_YELLOW | NS_YELLOW);
      HAL_Delay(1000);
    }
  }
  HAL_GPIO_WritePin(GPIOx, EW_GREEN | EW_YELLOW | EW_RED | NS_GREEN | NS_YELLOW | NS_RED, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOx, lights, GPIO_PIN_SET);
}

void Update_Traffic_Lights()
{
  int32_t delta = (int32_t)EW_traffic_cnt - (int32_t)NS_traffic_cnt;

  if (delta > 5)
  {
    consecutive_ns_high_traffic = 0;
    if (current_tier == TIER_0_BASE)
    {
      current_tier = TIER_1_EW_75;
      consecutive_ew_high_traffic = 1;
    }
    else if (current_tier == TIER_1_EW_75)
    {
      consecutive_ew_high_traffic++;
      if (consecutive_ew_high_traffic >= 2)
        current_tier = TIER_2_EW_90;
    }
    else if (current_tier == TIER_2_NS_90)
      current_tier = TIER_1_NS_75;
    else if (current_tier == TIER_1_NS_75)
      current_tier = TIER_0_BASE;
  }
  else if (delta < -5)
  {
    consecutive_ew_high_traffic = 0;
    if (current_tier == TIER_0_BASE)
    {
      current_tier = TIER_1_NS_75;
      consecutive_ns_high_traffic = 1;
    }
    else if (current_tier == TIER_1_NS_75)
    {
      consecutive_ns_high_traffic++;
      if (consecutive_ns_high_traffic >= 2)
        current_tier = TIER_2_NS_90;
    }
    else if (current_tier == TIER_2_EW_90)
      current_tier = TIER_1_EW_75;
    else if (current_tier == TIER_1_EW_75)
      current_tier = TIER_0_BASE;
  }
  else
  {
    consecutive_ew_high_traffic = 0;
    consecutive_ns_high_traffic = 0;
    if (current_tier == TIER_2_EW_90)
      current_tier = TIER_1_EW_75;
    else if (current_tier == TIER_1_EW_75)
      current_tier = TIER_0_BASE;
    else if (current_tier == TIER_2_NS_90)
      current_tier = TIER_1_NS_75;
    else if (current_tier == TIER_1_NS_75)
      current_tier = TIER_0_BASE;
  }

  switch (current_tier)
  {
  case TIER_0_BASE:
    green_duration_ew = 60000;
    green_duration_ns = 60000;
    break;
  case TIER_1_EW_75:
    green_duration_ew = 75000;
    green_duration_ns = 45000;
    break;
  case TIER_2_EW_90:
    green_duration_ew = 90000;
    green_duration_ns = 30000;
    break;
  case TIER_1_NS_75:
    green_duration_ew = 45000;
    green_duration_ns = 75000;
    break;
  case TIER_2_NS_90:
    green_duration_ew = 30000;
    green_duration_ns = 90000;
    break;
  }

  EW_traffic_cnt = 0;
  NS_traffic_cnt = 0;
}

int main()
{
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
  UART2_Init();
  I2C_Init();
  LCD_Init();

  uint32_t last_tick = HAL_GetTick();
  uint32_t last_seconds_tick = HAL_GetTick();
  uint32_t ew_seconds_remaining = 0, ns_seconds_remaining = 0;
  uint8_t current_state = STATE_ALL_RED_1;
  uint8_t state_changed = 1;

  while (1)
  {
    switch (current_state)
    {
    case STATE_ALL_RED_1:
      if (HAL_GetTick() - last_tick >= BASE_YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_EW_GREEN;
        state_changed = 1;
      }
      break;
    case STATE_EW_GREEN:
      if (HAL_GetTick() - last_tick >= green_duration_ew)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_EW_YELLOW;
        state_changed = 1;
      }
      break;
    case STATE_EW_YELLOW:
      if (HAL_GetTick() - last_tick >= BASE_YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_ALL_RED_2;
        state_changed = 1;
      }
      break;
    case STATE_ALL_RED_2:
      if (HAL_GetTick() - last_tick >= BASE_YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_NS_GREEN;
        state_changed = 1;
      }
      break;
    case STATE_NS_GREEN:
      if (HAL_GetTick() - last_tick >= green_duration_ns)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_NS_YELLOW;
        state_changed = 1;
      }
      break;
    case STATE_NS_YELLOW:
      if (HAL_GetTick() - last_tick >= BASE_YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        Update_Traffic_Lights();
        current_state = STATE_ALL_RED_1;
        state_changed = 1;
      }
      break;
    }

    if (state_changed)
    {
      switch (current_state)
      {
      case STATE_ALL_RED_1:
        Set_Traffic_Lights(GPIOA, EW_RED | NS_RED);
        ew_seconds_remaining = BASE_YELLOW_RED_DURATION_MS / 1000;
        ns_seconds_remaining = (green_duration_ew + 3 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_EW_GREEN:
        Set_Traffic_Lights(GPIOA, EW_GREEN | NS_RED);
        ew_seconds_remaining = green_duration_ew / 1000;
        ns_seconds_remaining = (green_duration_ew + 2 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_EW_YELLOW:
        Set_Traffic_Lights(GPIOA, EW_YELLOW | NS_RED);
        ew_seconds_remaining = BASE_YELLOW_RED_DURATION_MS / 1000;
        ns_seconds_remaining = (2 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_ALL_RED_2:
        Set_Traffic_Lights(GPIOA, EW_RED | NS_RED);
        ew_seconds_remaining = (green_duration_ns + 3 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = BASE_YELLOW_RED_DURATION_MS / 1000;
        break;
      case STATE_NS_GREEN:
        Set_Traffic_Lights(GPIOA, EW_RED | NS_GREEN);
        ew_seconds_remaining = (green_duration_ns + 2 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = green_duration_ns / 1000;
        break;
      case STATE_NS_YELLOW:
        Set_Traffic_Lights(GPIOA, EW_RED | NS_YELLOW);
        ew_seconds_remaining = (2 * BASE_YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = BASE_YELLOW_RED_DURATION_MS / 1000;
        break;
      }
      state_changed = 0;
    }

    if (HAL_GetTick() - last_seconds_tick >= 1000)
    {
      last_seconds_tick = HAL_GetTick();
      ew_seconds_remaining--, ns_seconds_remaining--;

      char buf[21];
      LCD_PrintLine(0, "SECONDS REMAINING:");

      snprintf(buf, sizeof(buf), "EW: %02lu   ||   NS: %02lu", ew_seconds_remaining, ns_seconds_remaining);
      LCD_PrintLine(1, buf);

      LCD_PrintLine(2, "CARS COUNT:");

      snprintf(buf, sizeof(buf), "EW: %02lu   ||   NS: %02lu", EW_traffic_cnt, NS_traffic_cnt);
      LCD_PrintLine(3, buf);

      printf("\rTime remaining -> EW:%02lus NS:%02lus || Cars Count -> EW:%02lu NS:%02lu",
        ew_seconds_remaining, ns_seconds_remaining, EW_traffic_cnt, NS_traffic_cnt);
      fflush(stdout);
    }
  }
  return 0;
}