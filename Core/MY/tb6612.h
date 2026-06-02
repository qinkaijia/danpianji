#ifndef __TB6612_H
#define __TB6612_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"

void TB6612_Init(void);
void TB6612_SetSpeed(int16_t speed);
void TB6612_Stop(void);
void TB6612_Brake(void);

#ifdef __cplusplus
}
#endif

#endif
