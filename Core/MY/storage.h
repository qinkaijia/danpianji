#ifndef __STORAGE_H
#define __STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define STORAGE_MAGIC    0x12345678

#define TEMP_THRESHOLD_MIN    20
#define TEMP_THRESHOLD_MAX    50
#define TEMP_THRESHOLD_DEFAULT 30

#define HUM_THRESHOLD_MIN     50
#define HUM_THRESHOLD_MAX     100
#define HUM_THRESHOLD_DEFAULT 80

#define DIST_THRESHOLD_MIN    5
#define DIST_THRESHOLD_MAX    100
#define DIST_THRESHOLD_DEFAULT 10

#define INTERVAL_MIN          1
#define INTERVAL_MAX          10
#define INTERVAL_DEFAULT      1

typedef struct {
    uint8_t temp_threshold;
    uint8_t hum_threshold;
    uint8_t dist_threshold;
    uint8_t record_interval;
    uint32_t magic;
} Storage_Params;

typedef struct {
    uint32_t timestamp;
    int16_t temp_x10;
    int16_t hum_x10;
} HistoryRecord;

#define HISTORY_SIZE    20

void Storage_Init(void);
void Storage_Load(Storage_Params *params);
void Storage_Save(const Storage_Params *params);

void History_Add(const HistoryRecord *rec);
uint8_t History_GetCount(void);
void History_GetRecord(uint8_t idx, HistoryRecord *rec);
void History_Clear(void);

#ifdef __cplusplus
}
#endif

#endif
