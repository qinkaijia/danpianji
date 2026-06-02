#include "pir.h"
#include "hcsr04.h"

static volatile uint8_t pir_motion_flag = 0;
static uint32_t pir_trigger_time = 0;

void PIR_Init(void)
{
    pir_motion_flag = 0;
    pir_trigger_time = 0;
}

uint8_t PIR_GetState(void)
{
    return HAL_GPIO_ReadPin(PIR_PORT, PIR_PIN);
}

uint8_t PIR_IsMotionDetected(void)
{
    if (pir_motion_flag) {
        return 1;
    }
    return 0;
}

void PIR_ClearMotionFlag(void)
{
    pir_motion_flag = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PIR_PIN) {
        if (HAL_GPIO_ReadPin(PIR_PORT, PIR_PIN) == GPIO_PIN_SET) {
            pir_motion_flag = 1;
            pir_trigger_time = HAL_GetTick();
        }
    }

    if (GPIO_Pin == HCSR04_ECHO_PIN) {
        HCSR04_EXTI_Handler();
    }
}
