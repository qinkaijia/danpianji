#include "esp8266.h"
#include "usart.h"
#include "app.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

volatile uint8_t esp_rx_buf[ESP_RX_BUF_SIZE];
volatile uint16_t esp_rx_idx = 0;
static uint8_t rx_byte;

typedef enum {
    ESP_S_RESET,
    ESP_S_WAIT_READY,
    ESP_S_MODE,
    ESP_S_WAIT_MODE,
    ESP_S_CONNECT,
    ESP_S_WAIT_CONNECT,
    ESP_S_MUX,
    ESP_S_WAIT_MUX,
    ESP_S_SERVER,
    ESP_S_WAIT_SERVER,
    ESP_S_READY,
    ESP_S_ERROR
} ESP_InitState;

static ESP_InitState esp_init_state = ESP_S_RESET;
static uint32_t esp_tick = 0;
static uint8_t esp_retry = 0;
static ESP8266_State esp_state = ESP_STATE_RESET;

static void ESP8266_SendRaw(const char *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 1000);
}

static void ESP8266_SendAT(const char *cmd)
{
    ESP8266_SendRaw(cmd, strlen(cmd));
    ESP8266_SendRaw("\r\n", 2);
}

static void ESP8266_ClearBuf(void)
{
    esp_rx_idx = 0;
}

static uint8_t ESP8266_HasResponse(const char *resp)
{
    if (esp_rx_idx > 0) {
        esp_rx_buf[esp_rx_idx] = '\0';
        return (strstr((char *)esp_rx_buf, resp) != NULL) ? 1 : 0;
    }
    return 0;
}

static void ESP8266_SendCmd(const char *cmd)
{
    ESP8266_ClearBuf();
    ESP8266_SendAT(cmd);
    esp_tick = HAL_GetTick();
}

void ESP8266_Init(void)
{
    esp_rx_idx = 0;
    esp_init_state = ESP_S_RESET;
    esp_state = ESP_STATE_RESET;
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        if (esp_rx_idx < ESP_RX_BUF_SIZE - 1) {
            esp_rx_buf[esp_rx_idx++] = rx_byte;
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

static void fmt_float(char *buf, float val)
{
    int x10 = (int)(val * 10.0f);
    int dec = abs(x10 % 10);
    sprintf(buf, "%d.%d", x10 / 10, dec);
}

static void ESP8266_ProcessRx(void)
{
    if (esp_rx_idx == 0) return;

    uint16_t len;
    uint8_t local_buf[ESP_RX_BUF_SIZE];

    __disable_irq();
    len = esp_rx_idx;
    memcpy(local_buf, (uint8_t *)esp_rx_buf, len);
    esp_rx_idx = 0;
    __enable_irq();

    local_buf[len] = '\0';

    char *ipd = strstr((char *)local_buf, "+IPD,");
    if (!ipd) return;

    int link_id = -1;
    int data_len = 0;
    if (sscanf(ipd, "+IPD,%d,%d:", &link_id, &data_len) < 1) return;
    if (link_id < 0) return;

    char *data_start = strchr(ipd, ':');
    if (!data_start) return;
    data_start++;

    if (!strstr(data_start, "GET /") && !strstr(data_start, "GET /HTTP")) return;

    char temp_str[12];
    char hum_str[12];
    fmt_float(temp_str, g_sensor.temperature);
    fmt_float(hum_str, g_sensor.humidity);

    char html[1400];
    int html_len = sprintf(html,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>环境监测</title>"
        "<style>"
        "body{font-family:sans-serif;background:#f0f0f0;padding:20px;margin:0;}"
        "h1{color:#333;text-align:center;}"
        ".card{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}"
        ".label{color:#666;font-size:14px;}"
        ".value{color:#333;font-size:32px;font-weight:bold;}"
        ".unit{font-size:16px;color:#666;}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>环境监测终端</h1>"
        "<div class=\"card\">"
        "<div class=\"label\">温度</div>"
        "<div class=\"value\">%s<span class=\"unit\">&deg;C</span></div>"
        "</div>"
        "<div class=\"card\">"
        "<div class=\"label\">湿度</div>"
        "<div class=\"value\">%s<span class=\"unit\">%%</span></div>"
        "</div>"
        "<div class=\"card\">"
        "<div class=\"label\">距离</div>"
        "<div class=\"value\">%d<span class=\"unit\">cm</span></div>"
        "</div>"
        "<div class=\"card\">"
        "<div class=\"label\">人体检测</div>"
        "<div class=\"value\">%s</div>"
        "</div>"
        "<div class=\"card\">"
        "<div class=\"label\">风扇转速</div>"
        "<div class=\"value\">%d<span class=\"unit\">%%</span></div>"
        "</div>"
        "</body>"
        "</html>",
        temp_str, hum_str,
        g_sensor.distance,
        g_sensor.motion ? "Detected" : "None",
        g_sensor.fan_speed);

    char cmd[32];
    sprintf(cmd, "AT+CIPSEND=%d,%d", link_id, html_len);
    ESP8266_SendCmd(cmd);
    HAL_Delay(50);
    ESP8266_SendRaw(html, html_len);
    HAL_Delay(100);
    sprintf(cmd, "AT+CIPCLOSE=%d", link_id);
    ESP8266_SendAT(cmd);
}

void ESP8266_Task(void)
{
    ESP8266_ProcessRx();

    switch (esp_init_state) {
        case ESP_S_RESET:
            ESP8266_SendCmd("AT+RST");
            esp_init_state = ESP_S_WAIT_READY;
            esp_tick = HAL_GetTick();
            esp_retry = 0;
            break;

        case ESP_S_WAIT_READY:
            if (ESP8266_HasResponse("ready")) {
                esp_init_state = ESP_S_MODE;
                esp_retry = 0;
            } else if (HAL_GetTick() - esp_tick > 3000) {
                esp_retry++;
                if (esp_retry > 3) {
                    esp_state = ESP_STATE_ERROR;
                    esp_init_state = ESP_S_ERROR;
                } else {
                    esp_init_state = ESP_S_RESET;
                }
            }
            break;

        case ESP_S_MODE:
            ESP8266_SendCmd("AT+CWMODE=1");
            esp_init_state = ESP_S_WAIT_MODE;
            break;

        case ESP_S_WAIT_MODE:
            if (ESP8266_HasResponse("OK")) {
                esp_init_state = ESP_S_CONNECT;
                esp_retry = 0;
            } else if (HAL_GetTick() - esp_tick > 2000) {
                esp_retry++;
                if (esp_retry > 3) {
                    esp_state = ESP_STATE_ERROR;
                    esp_init_state = ESP_S_ERROR;
                } else {
                    esp_init_state = ESP_S_MODE;
                }
            }
            break;

        case ESP_S_CONNECT: {
            char cmd[80];
            sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ESP8266_SSID, ESP8266_PASS);
            ESP8266_SendCmd(cmd);
            esp_init_state = ESP_S_WAIT_CONNECT;
            break;
        }

        case ESP_S_WAIT_CONNECT:
            if (ESP8266_HasResponse("WIFI GOT IP") || ESP8266_HasResponse("OK")) {
                esp_init_state = ESP_S_MUX;
                esp_retry = 0;
            } else if (HAL_GetTick() - esp_tick > 15000) {
                esp_retry++;
                if (esp_retry > 3) {
                    esp_state = ESP_STATE_ERROR;
                    esp_init_state = ESP_S_ERROR;
                } else {
                    esp_init_state = ESP_S_CONNECT;
                }
            }
            break;

        case ESP_S_MUX:
            ESP8266_SendCmd("AT+CIPMUX=1");
            esp_init_state = ESP_S_WAIT_MUX;
            break;

        case ESP_S_WAIT_MUX:
            if (ESP8266_HasResponse("OK")) {
                esp_init_state = ESP_S_SERVER;
                esp_retry = 0;
            } else if (HAL_GetTick() - esp_tick > 2000) {
                esp_retry++;
                if (esp_retry > 3) {
                    esp_state = ESP_STATE_ERROR;
                    esp_init_state = ESP_S_ERROR;
                } else {
                    esp_init_state = ESP_S_MUX;
                }
            }
            break;

        case ESP_S_SERVER:
            ESP8266_SendCmd("AT+CIPSERVER=1,80");
            esp_init_state = ESP_S_WAIT_SERVER;
            break;

        case ESP_S_WAIT_SERVER:
            if (ESP8266_HasResponse("OK")) {
                esp_init_state = ESP_S_READY;
                esp_state = ESP_STATE_SERVER;
                esp_retry = 0;
            } else if (HAL_GetTick() - esp_tick > 2000) {
                esp_retry++;
                if (esp_retry > 3) {
                    esp_state = ESP_STATE_ERROR;
                    esp_init_state = ESP_S_ERROR;
                } else {
                    esp_init_state = ESP_S_SERVER;
                }
            }
            break;

        case ESP_S_READY:
            break;

        case ESP_S_ERROR:
            if (HAL_GetTick() - esp_tick > 10000) {
                esp_init_state = ESP_S_RESET;
                esp_state = ESP_STATE_RESET;
            }
            break;
    }
}

ESP8266_State ESP8266_GetState(void)
{
    return esp_state;
}
