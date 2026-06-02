#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define DHT11_PORT    GPIOA
#define DHT11_PIN     GPIO_PIN_8

void DHT11_Init(void);
int DHT11_Read(float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

#endif
