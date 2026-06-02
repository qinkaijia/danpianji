#include "hcsr04.h"

static volatile uint8_t hcsr04_measuring = 0;
static volatile uint16_t hcsr04_start = 0;
static volatile uint16_t hcsr04_end = 0;
static volatile uint8_t hcsr04_ready = 0;

void HCSR04_Init(void)
{
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);
    HAL_TIM_Base_Start(&htim4);
}

void HCSR04_EXTI_Handler(void)
{
    if (!hcsr04_measuring) {
        hcsr04_start = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
        hcsr04_measuring = 1;
    } else {
        hcsr04_end = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
        hcsr04_ready = 1;
        hcsr04_measuring = 0;
    }
}

int16_t HCSR04_ReadCm(void)
{
    hcsr04_ready = 0;
    hcsr04_measuring = 0;

    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_SET);
    uint16_t t0 = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
    while ((uint16_t)(__HAL_TIM_GET_COUNTER(&htim4) - t0) < 15);
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);

    uint16_t timeout_start = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
    while (!hcsr04_ready) {
        if ((uint16_t)(__HAL_TIM_GET_COUNTER(&htim4) - timeout_start) > 30000) {
            return -1;
        }
    }

    uint16_t duration_us = (uint16_t)(hcsr04_end - hcsr04_start);
    int16_t distance = (int16_t)(duration_us / 58);
    if (distance < 2 || distance > 400) return -1;
    return distance;
}
