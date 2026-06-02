#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define LED_GREEN_PIN    GPIO_PIN_4
#define LED_GREEN_PORT   GPIOA

#define LED_YELLOW_PIN   GPIO_PIN_5
#define LED_YELLOW_PORT  GPIOA

#define LED_RED_PIN      GPIO_PIN_6
#define LED_RED_PORT     GPIOA

void LED_Init(void);
void LED_Green(uint8_t state);
void LED_Yellow(uint8_t state);
void LED_Red(uint8_t state);
void LED_RedToggle(void);

#ifdef __cplusplus
}
#endif

#endif
