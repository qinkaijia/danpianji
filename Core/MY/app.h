#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    float temperature;
    float humidity;
    int16_t distance;
    uint8_t motion;
    uint8_t fan_speed;
    uint8_t alarm;
} App_SensorData;

extern App_SensorData g_sensor;

void App_Init(void);
void App_Run(void);

#ifdef __cplusplus
}
#endif

#endif
