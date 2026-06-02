#ifndef __HCSR04_H
#define __HCSR04_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"

#define HCSR04_TRIG_PORT    GPIOA
#define HCSR04_TRIG_PIN     GPIO_PIN_2
#define HCSR04_ECHO_PORT    GPIOA
#define HCSR04_ECHO_PIN     GPIO_PIN_3

void HCSR04_Init(void);
void HCSR04_EXTI_Handler(void);
int16_t HCSR04_ReadCm(void);

#ifdef __cplusplus
}
#endif

#endif
