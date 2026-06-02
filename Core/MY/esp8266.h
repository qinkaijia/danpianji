#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ESP8266_SSID     "ningkai"
#define ESP8266_PASS     "0502080317"

#define ESP_RX_BUF_SIZE  512

typedef enum {
    ESP_STATE_RESET,
    ESP_STATE_INIT,
    ESP_STATE_CONNECTING,
    ESP_STATE_SERVER,
    ESP_STATE_ERROR
} ESP8266_State;

void ESP8266_Init(void);
void ESP8266_Task(void);
ESP8266_State ESP8266_GetState(void);

void ESP8266_SendData(const char *data, uint16_t len);

extern volatile uint8_t esp_data_ready;
extern volatile uint8_t esp_rx_buf[ESP_RX_BUF_SIZE];
extern volatile uint16_t esp_rx_len;

#ifdef __cplusplus
}
#endif

#endif
