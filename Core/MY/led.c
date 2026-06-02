#include "led.h"

void LED_Init(void)
{
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
}

void LED_Green(uint8_t state)
{
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_Yellow(uint8_t state)
{
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_Red(uint8_t state)
{
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_RedToggle(void)
{
    HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
}
