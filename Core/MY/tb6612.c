#include "tb6612.h"

void TB6612_Init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    TB6612_Stop();
}

void TB6612_SetSpeed(int16_t speed)
{
    if (speed > 100) {
        speed = 100;
    } else if (speed < -100) {
        speed = -100;
    }

    if (speed > 0) {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (uint16_t)(speed * 65535 / 100));
    } else if (speed < 0) {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (uint16_t)((-speed) * 65535 / 100));
    } else {
        TB6612_Stop();
    }
}

void TB6612_Stop(void)
{
    HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
}

void TB6612_Brake(void)
{
    HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
}
