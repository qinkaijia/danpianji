#ifndef __PIR_H
#define __PIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define PIR_PIN       GPIO_PIN_5
#define PIR_PORT      GPIOB

void PIR_Init(void);
uint8_t PIR_GetState(void);
uint8_t PIR_IsMotionDetected(void);
void PIR_ClearMotionFlag(void);

#ifdef __cplusplus
}
#endif

#endif
