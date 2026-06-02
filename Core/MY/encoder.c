#include "encoder.h"
#include "tim.h"

static int16_t last_count = 0;

void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    last_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
}

int16_t Encoder_GetCount(void)
{
    return (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
}

int16_t Encoder_GetDelta(void)
{
    int16_t current = Encoder_GetCount();
    int16_t delta = current - last_count;
    last_count = current;
    return delta;
}

void Encoder_ClearCount(void)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    last_count = 0;
}

uint8_t Encoder_IsPressed(void)
{
    if (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) == GPIO_PIN_RESET) {
            while (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) == GPIO_PIN_RESET);
            return 1;
        }
    }
    return 0;
}
