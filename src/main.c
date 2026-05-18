#include "defines.h"
#include "setup.h"

#include <stdio.h>

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

int main()
{
  HAL_Init();
  GPIO_Init();
  UART2_Init();

  uint32_t last_tick = HAL_GetTick();
  uint32_t last_seconds_tick = HAL_GetTick();
  uint32_t ew_seconds_remaining = 0, ns_seconds_remaining = 0;
  uint8_t current_state = STATE_ALL_RED_1;
  uint8_t state_changed = 1;

  printf("\n\n==============================================================\n");
      printf("Traffic monitoring system is live. GPIO and UART2 are active.\n");
      printf("==============================================================\n");
      
  while (1)
  {
    switch (current_state)
    {
    case STATE_ALL_RED_1:
      if (HAL_GetTick() - last_tick >= YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_EW_GREEN;
        state_changed = 1;
      }
      break;
    case STATE_EW_GREEN:
      if (HAL_GetTick() - last_tick >= GREEN_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_EW_YELLOW;
        state_changed = 1;
      }
      break;
    case STATE_EW_YELLOW:
      if (HAL_GetTick() - last_tick >= YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_ALL_RED_2;
        state_changed = 1;
      }
      break;
    case STATE_ALL_RED_2:
      if (HAL_GetTick() - last_tick >= YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_NS_GREEN;
        state_changed = 1;
      }
      break;
    case STATE_NS_GREEN:
      if (HAL_GetTick() - last_tick >= GREEN_DURATION_MS)
      {
        last_tick = HAL_GetTick();
        current_state = STATE_NS_YELLOW;
        state_changed = 1;
      }
      break;
    case STATE_NS_YELLOW:
      if (HAL_GetTick() - last_tick >= YELLOW_RED_DURATION_MS)
      {
        last_tick = HAL_GetTick();
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
        printf("\nALL RED for %d ms.\n", YELLOW_RED_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_RED | NS_RED);
        ew_seconds_remaining = YELLOW_RED_DURATION_MS / 1000;
        ns_seconds_remaining = (GREEN_DURATION_MS + 3 * YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_EW_GREEN:
        printf("\nEAST_WEST green for %d ms.\n", GREEN_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_GREEN | NS_RED);
        ew_seconds_remaining = GREEN_DURATION_MS / 1000;
        ns_seconds_remaining = (GREEN_DURATION_MS + 2 * YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_EW_YELLOW:
        printf("\nEAST_WEST yellow for %d ms.\n", YELLOW_RED_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_YELLOW | NS_RED);
        ew_seconds_remaining = YELLOW_RED_DURATION_MS / 1000;
        ns_seconds_remaining = (2 * YELLOW_RED_DURATION_MS) / 1000;
        break;
      case STATE_ALL_RED_2:
        printf("\nALL RED for %d ms.\n", YELLOW_RED_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_RED | NS_RED);
        ew_seconds_remaining = (GREEN_DURATION_MS + 3 * YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = YELLOW_RED_DURATION_MS / 1000;
        break;
      case STATE_NS_GREEN:
        printf("\nNORTH_SOUTH green for %d ms.\n", GREEN_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_RED | NS_GREEN);
        ew_seconds_remaining = (GREEN_DURATION_MS + 2 * YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = GREEN_DURATION_MS / 1000;
        break;
      case STATE_NS_YELLOW:
        printf("\nNORTH_SOUTH yellow for %d ms.\n", YELLOW_RED_DURATION_MS);
        Set_Traffic_Lights(GPIOA, EW_RED | NS_YELLOW);
        ew_seconds_remaining = (2 * YELLOW_RED_DURATION_MS) / 1000;
        ns_seconds_remaining = YELLOW_RED_DURATION_MS / 1000;
        break;
      }
      state_changed = 0;
    }

    if (HAL_GetTick() - last_seconds_tick >= 1000)
    {
      last_seconds_tick = HAL_GetTick();
      ew_seconds_remaining--, ns_seconds_remaining--;
      printf("\rEW seconds remaining: %lu. NS seconds remaining: %lu  ", ew_seconds_remaining, ns_seconds_remaining);
      fflush(stdout);
    }
  }
  return 0;
}