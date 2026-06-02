#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define ENCODER_SW_PIN       GPIO_PIN_3
#define ENCODER_SW_PORT      GPIOB

void Encoder_Init(void);
int16_t Encoder_GetCount(void);
int16_t Encoder_GetDelta(void);
void Encoder_ClearCount(void);
uint8_t Encoder_IsPressed(void);

#ifdef __cplusplus
}
#endif

#endif
