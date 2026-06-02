#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define BUZZER_PIN       GPIO_PIN_4
#define BUZZER_PORT      GPIOB

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Toggle(void);
void Buzzer_Beep(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif
