#include "app.h"
#include "main.h"
#include "oled.h"
#include "encoder.h"
#include "key.h"
#include "led.h"
#include "buzzer.h"
#include "pir.h"
#include "dht11.h"
#include "hcsr04.h"
#include "tb6612.h"
#include "storage.h"
#include "esp8266.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

App_SensorData g_sensor = {0};

typedef struct {
    float temp;
    float hum;
    int16_t dist;
    uint8_t motion;
} LocalSensor;

typedef enum {
    STATE_DASHBOARD,
    STATE_MENU,
    STATE_EDIT,
    STATE_HISTORY,
    STATE_CONFIRM
} AppState;

static LocalSensor sensor = {0};
static uint8_t fan_speed = 0;
static uint8_t fan_manual = 0;
static uint8_t fan_manual_speed = 50;

static Storage_Params params;
static AppState app_state = STATE_DASHBOARD;
static uint8_t menu_index = 0;

static uint32_t tick_dht11 = 0;
static uint32_t tick_dist = 0;
static uint32_t tick_display = 0;
static uint32_t tick_pir_led = 0;
static uint32_t tick_activity = 0;
static uint32_t tick_sec = 0;
static uint32_t sys_tick_sec = 0;

static uint8_t oled_sleep = 0;
static uint8_t edit_value = 0;
static uint8_t hist_view_idx = 0;

static uint8_t alarm_dist = 0;
static uint8_t alarm_hum = 0;
static uint8_t alarm_temp = 0;
static uint32_t pir_beep_tick = 0;
static uint32_t tick_input = 0;
static int8_t enc_last_dir = 0;
static uint8_t enc_same_cnt = 0;

static const char *menu_items[] = {
    "Temp Threshold",
    "Hum Threshold",
    "Dist Threshold",
    "Rec Interval",
    "Fan Manual",
    "History",
    "Clear History",
    "Back"
};
#define MENU_COUNT 8

static void ReadSensors(void);
static void UpdateControl(void);
static void UpdateLED(void);
static void UpdateBuzzer(void);
static void RenderDashboard(void);
static void RenderMenu(void);
static void RenderEdit(void);
static void RenderHistory(void);
static void RenderConfirm(void);
static void HandleInput(void);
static void SaveRecord(void);
static void UpdateSensorGlobal(void);

void App_Init(void)
{
    LED_Init();
    Key_Init();
    Storage_Init();
    Storage_Load(&params);
    ESP8266_Init();

    LED_Green(1);
    tick_activity = HAL_GetTick();
}

void App_Run(void)
{
    ReadSensors();
    UpdateControl();
    UpdateLED();
    UpdateBuzzer();
    HandleInput();

    if (HAL_GetTick() - tick_display >= 100) {
        tick_display = HAL_GetTick();
        if (!oled_sleep) {
            switch (app_state) {
                case STATE_DASHBOARD: RenderDashboard(); break;
                case STATE_MENU:      RenderMenu();      break;
                case STATE_EDIT:      RenderEdit();      break;
                case STATE_HISTORY:   RenderHistory();   break;
                case STATE_CONFIRM:   RenderConfirm();   break;
            }
        }
    }

    ESP8266_Task();

    if (HAL_GetTick() - tick_sec >= 1000) {
        tick_sec = HAL_GetTick();
        sys_tick_sec++;
        if (params.record_interval > 0 &&
            (sys_tick_sec % (params.record_interval * 60U)) == 0) {
            SaveRecord();
        }
    }

    if (HAL_GetTick() - tick_activity > 10000) {
        if (!oled_sleep) {
            oled_sleep = 1;
            OLED_Clear();
            OLED_Refresh();
        }
    }

    UpdateSensorGlobal();
}

static void UpdateSensorGlobal(void)
{
    g_sensor.temperature = sensor.temp;
    g_sensor.humidity = sensor.hum;
    g_sensor.distance = sensor.dist;
    g_sensor.motion = sensor.motion;
    g_sensor.fan_speed = fan_speed;
    g_sensor.alarm = (alarm_temp || alarm_hum || alarm_dist) ? 1 : 0;
}

static void ReadSensors(void)
{
    if (HAL_GetTick() - tick_dht11 >= 2000) {
        tick_dht11 = HAL_GetTick();
        float t, h;
        if (DHT11_Read(&t, &h) == 0) {
            sensor.temp = t;
            sensor.hum = h;
        }
    }

    if (HAL_GetTick() - tick_dist >= 200) {
        tick_dist = HAL_GetTick();
        sensor.dist = HCSR04_ReadCm();
    }

    sensor.motion = PIR_IsMotionDetected();
    if (sensor.motion) {
        PIR_ClearMotionFlag();
    }
}

static void UpdateControl(void)
{
    if (fan_manual) {
        TB6612_SetSpeed((int16_t)(fan_manual_speed * 10));
        fan_speed = fan_manual_speed;
    } else {
        if (sensor.temp > params.temp_threshold) {
            uint8_t spd = (uint8_t)((sensor.temp - params.temp_threshold) * 10.0f);
            if (spd > 100) spd = 100;
            TB6612_SetSpeed((int16_t)(spd * 10));
            fan_speed = spd;
        } else {
            TB6612_Stop();
            fan_speed = 0;
        }
    }

    alarm_temp = (sensor.temp > params.temp_threshold) ? 1 : 0;
    alarm_hum  = (sensor.hum > params.hum_threshold) ? 1 : 0;
    alarm_dist = (sensor.dist >= 0 && sensor.dist < params.dist_threshold) ? 1 : 0;
}

static void UpdateLED(void)
{
    if (sensor.motion) {
        LED_Yellow(1);
        tick_pir_led = HAL_GetTick();
    } else if (HAL_GetTick() - tick_pir_led > 1000) {
        LED_Yellow(0);
    }

    if (alarm_temp || alarm_hum || alarm_dist) {
        LED_Red(((HAL_GetTick() / 500) & 1) ? 1 : 0);
    } else {
        LED_Red(0);
    }
}

static void UpdateBuzzer(void)
{
    if (alarm_dist) {
        uint32_t phase = HAL_GetTick() % 200;
        if (phase < 100) Buzzer_On(); else Buzzer_Off();
    } else if (alarm_hum) {
        uint32_t phase = HAL_GetTick() % 1000;
        if (phase < 100) Buzzer_On(); else Buzzer_Off();
    } else if (sensor.motion && (HAL_GetTick() - pir_beep_tick > 2000)) {
        pir_beep_tick = HAL_GetTick();
        Buzzer_Beep(50);
    } else {
        Buzzer_Off();
    }
}

static void fmt_temp(char *buf, float val)
{
    int x10 = (int)(val * 10.0f);
    int dec = abs(x10 % 10);
    sprintf(buf, "%d.%d", x10 / 10, dec);
}

static void fmt_hum(char *buf, float val)
{
    int x10 = (int)(val * 10.0f);
    int dec = abs(x10 % 10);
    sprintf(buf, "%d.%d", x10 / 10, dec);
}

static void RenderDashboard(void)
{
    char buf[22];
    OLED_Clear();

    fmt_temp(buf, sensor.temp);
    char line0[32];
    sprintf(line0, "T:%sC", buf);
    fmt_hum(buf, sensor.hum);
    sprintf(line0 + strlen(line0), " H:%s%%", buf);
    OLED_ShowString(0, 0, line0, 1);

    if (sensor.dist >= 0) {
        sprintf(buf, "Dist:%dcm", sensor.dist);
    } else {
        sprintf(buf, "Dist:ERR");
    }
    OLED_ShowString(0, 16, buf, 1);

    sprintf(buf, "Motion:%s", sensor.motion ? "YES" : "NO");
    OLED_ShowString(0, 32, buf, 1);

    char alm = (alarm_temp || alarm_hum || alarm_dist) ? 'A' : 'N';
    sprintf(buf, "Fan:%d%% Alm:%c", fan_speed, alm);
    OLED_ShowString(0, 48, buf, 1);

    OLED_Refresh();
}

static void RenderMenu(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "Menu", 1);

    for (int i = 0; i < 3; i++) {
        int idx = menu_index + i;
        if (idx >= MENU_COUNT) break;

        char buf[22];
        if (i == 0) {
            sprintf(buf, "> %s", menu_items[idx]);
        } else {
            sprintf(buf, "  %s", menu_items[idx]);
        }
        OLED_ShowString(0, 16 + i * 16, buf, 1);
    }
    OLED_Refresh();
}

static void RenderEdit(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, menu_items[menu_index], 1);

    char buf[22];
    switch (menu_index) {
        case 0: sprintf(buf, "Val:%dC",   edit_value); break;
        case 1: sprintf(buf, "Val:%d%%",  edit_value); break;
        case 2: sprintf(buf, "Val:%dcm",  edit_value); break;
        case 3: sprintf(buf, "Val:%dmin", edit_value); break;
        case 4: sprintf(buf, "Spd:%d%%",  edit_value); break;
        default: buf[0] = '\0'; break;
    }
    OLED_ShowString(0, 24, buf, 1);
    OLED_ShowString(0, 48, "Rot=Adj SW=Save", 1);
    OLED_Refresh();
}

static void RenderHistory(void)
{
    OLED_Clear();
    uint8_t cnt = History_GetCount();

    if (cnt == 0) {
        OLED_ShowString(0, 0, "No History", 1);
        OLED_ShowString(0, 48, "KEY=Back", 1);
    } else {
        HistoryRecord rec;
        History_GetRecord(hist_view_idx, &rec);

        char buf[22];
        sprintf(buf, "%d/%d", hist_view_idx + 1, cnt);
        OLED_ShowString(0, 0, buf, 1);

        int t_dec = abs(rec.temp_x10 % 10);
        int h_dec = abs(rec.hum_x10 % 10);
        sprintf(buf, "T:%d.%d H:%d.%d",
                rec.temp_x10 / 10, t_dec,
                rec.hum_x10 / 10, h_dec);
        OLED_ShowString(0, 16, buf, 1);

        sprintf(buf, "T:%lus", (unsigned long)rec.timestamp);
        OLED_ShowString(0, 32, buf, 1);
        OLED_ShowString(0, 48, "Rot=Page KEY=Back", 1);
    }
    OLED_Refresh();
}

static void RenderConfirm(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "Clear History?", 1);
    OLED_ShowString(0, 24, "SW=Yes KEY=No", 1);
    OLED_Refresh();
}

static void HandleInput(void)
{
    int16_t delta = 0;
    uint8_t sw = Encoder_IsPressed();
    uint8_t key = Key_IsPressed();

    if (HAL_GetTick() - tick_input >= 20) {
        tick_input = HAL_GetTick();
        int16_t raw = Encoder_GetDelta();

        int8_t dir = 0;
        if (raw >= 3) dir = 1;
        else if (raw <= -3) dir = -1;

        if (dir == enc_last_dir && dir != 0) {
            enc_same_cnt++;
            if (enc_same_cnt >= 2) {
                delta = dir;
                enc_same_cnt = 0;
            }
        } else {
            enc_same_cnt = (dir != 0) ? 1 : 0;
        }
        enc_last_dir = dir;
    }

    if (delta != 0 || sw || key) {
        tick_activity = HAL_GetTick();
        if (oled_sleep) {
            oled_sleep = 0;
            return;
        }
    }

    switch (app_state) {
        case STATE_DASHBOARD:
            if (sw) {
                app_state = STATE_MENU;
                menu_index = 0;
            }
            if (key && fan_manual) {
                fan_manual = 0;
                fan_manual_speed = 0;
                TB6612_Stop();
                fan_speed = 0;
            }
            break;

        case STATE_MENU:
            if (delta > 0 && menu_index < MENU_COUNT - 1) menu_index++;
            if (delta < 0 && menu_index > 0) menu_index--;
            if (sw) {
                if (menu_index == MENU_COUNT - 1) {
                    app_state = STATE_DASHBOARD;
                } else if (menu_index == 5) {
                    app_state = STATE_HISTORY;
                    hist_view_idx = 0;
                } else if (menu_index == 6) {
                    app_state = STATE_CONFIRM;
                } else if (menu_index == 4) {
                    app_state = STATE_EDIT;
                    edit_value = fan_manual_speed;
                } else {
                    app_state = STATE_EDIT;
                    switch (menu_index) {
                        case 0: edit_value = params.temp_threshold; break;
                        case 1: edit_value = params.hum_threshold;  break;
                        case 2: edit_value = params.dist_threshold; break;
                        case 3: edit_value = params.record_interval; break;
                    }
                }
            }
            if (key) {
                app_state = STATE_DASHBOARD;
            }
            break;

        case STATE_EDIT:
            if (delta != 0) {
                int16_t v = (int16_t)edit_value + delta;
                switch (menu_index) {
                    case 0:
                        if (v < TEMP_THRESHOLD_MIN) v = TEMP_THRESHOLD_MIN;
                        if (v > TEMP_THRESHOLD_MAX) v = TEMP_THRESHOLD_MAX;
                        break;
                    case 1:
                        if (v < HUM_THRESHOLD_MIN) v = HUM_THRESHOLD_MIN;
                        if (v > HUM_THRESHOLD_MAX) v = HUM_THRESHOLD_MAX;
                        break;
                    case 2:
                        if (v < DIST_THRESHOLD_MIN) v = DIST_THRESHOLD_MIN;
                        if (v > DIST_THRESHOLD_MAX) v = DIST_THRESHOLD_MAX;
                        break;
                    case 3:
                        if (v < INTERVAL_MIN) v = INTERVAL_MIN;
                        if (v > INTERVAL_MAX) v = INTERVAL_MAX;
                        break;
                    case 4:
                        if (v < 0) v = 0;
                        if (v > 100) v = 100;
                        break;
                }
                edit_value = (uint8_t)v;
            }
            if (sw) {
                switch (menu_index) {
                    case 0: params.temp_threshold = edit_value; break;
                    case 1: params.hum_threshold  = edit_value; break;
                    case 2: params.dist_threshold = edit_value; break;
                    case 3: params.record_interval = edit_value; break;
                    case 4:
                        fan_manual_speed = edit_value;
                        fan_manual = (edit_value > 0);
                        break;
                }
                if (menu_index != 4) {
                    Storage_Save(&params);
                }
                app_state = STATE_MENU;
            }
            if (key) {
                app_state = STATE_MENU;
            }
            break;

        case STATE_HISTORY:
            if (delta > 0 && hist_view_idx + 1 < History_GetCount()) hist_view_idx++;
            if (delta < 0 && hist_view_idx > 0) hist_view_idx--;
            if (sw || key) {
                app_state = STATE_MENU;
            }
            break;

        case STATE_CONFIRM:
            if (sw) {
                History_Clear();
                app_state = STATE_MENU;
            }
            if (key) {
                app_state = STATE_MENU;
            }
            break;
    }
}

static void SaveRecord(void)
{
    HistoryRecord rec;
    rec.timestamp = sys_tick_sec;
    rec.temp_x10 = (int16_t)(sensor.temp * 10.0f);
    rec.hum_x10  = (int16_t)(sensor.hum  * 10.0f);
    History_Add(&rec);
}
