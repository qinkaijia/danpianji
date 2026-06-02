#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define KEY_PIN       GPIO_PIN_8
#define KEY_PORT      GPIOB

void Key_Init(void);
uint8_t Key_IsPressed(void);

#ifdef __cplusplus
}
#endif

#endif
