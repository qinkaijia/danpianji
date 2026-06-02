#include "storage.h"
#include "w25q64.h"
#include <string.h>

#define PARAM_ADDR      0x000000
#define HISTORY_ADDR    0x001000

static HistoryRecord history_buffer[HISTORY_SIZE];
static uint8_t history_count = 0;
static uint8_t history_head = 0;
static uint16_t flash_record_count = 0;

static uint8_t IsEmptyBuf(const uint8_t *buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if (buf[i] != 0xFF) return 0;
    }
    return 1;
}

void Storage_Init(void)
{
    uint8_t buf[8];
    flash_record_count = 0;
    for (uint32_t addr = HISTORY_ADDR + 4;
         addr < HISTORY_ADDR + W25Q64_SECTOR_SIZE;
         addr += 8) {
        W25Q64_Read(addr, buf, 8);
        if (IsEmptyBuf(buf, 8)) break;
        flash_record_count++;
    }

    uint16_t load_count =
        (flash_record_count < HISTORY_SIZE) ? flash_record_count : HISTORY_SIZE;
    uint32_t start_addr =
        HISTORY_ADDR + 4 + (flash_record_count - load_count) * 8;

    history_count = load_count;
    history_head = load_count % HISTORY_SIZE;
    for (uint16_t i = 0; i < load_count; i++) {
        W25Q64_Read(start_addr + i * 8, (uint8_t *)&history_buffer[i], 8);
    }
}

void Storage_Load(Storage_Params *params)
{
    uint8_t buf[16];
    W25Q64_Read(PARAM_ADDR, buf, sizeof(buf));
    memcpy(params, buf, sizeof(Storage_Params));

    if (params->magic != STORAGE_MAGIC) {
        params->temp_threshold = TEMP_THRESHOLD_DEFAULT;
        params->hum_threshold  = HUM_THRESHOLD_DEFAULT;
        params->dist_threshold = DIST_THRESHOLD_DEFAULT;
        params->record_interval = INTERVAL_DEFAULT;
        params->magic = STORAGE_MAGIC;
        Storage_Save(params);
    }

    if (params->temp_threshold < TEMP_THRESHOLD_MIN ||
        params->temp_threshold > TEMP_THRESHOLD_MAX)
        params->temp_threshold = TEMP_THRESHOLD_DEFAULT;
    if (params->hum_threshold < HUM_THRESHOLD_MIN ||
        params->hum_threshold > HUM_THRESHOLD_MAX)
        params->hum_threshold = HUM_THRESHOLD_DEFAULT;
    if (params->dist_threshold < DIST_THRESHOLD_MIN ||
        params->dist_threshold > DIST_THRESHOLD_MAX)
        params->dist_threshold = DIST_THRESHOLD_DEFAULT;
    if (params->record_interval < INTERVAL_MIN ||
        params->record_interval > INTERVAL_MAX)
        params->record_interval = INTERVAL_DEFAULT;
}

void Storage_Save(const Storage_Params *params)
{
    uint8_t buf[16];
    memcpy(buf, params, sizeof(Storage_Params));
    W25Q64_EraseSector(PARAM_ADDR);
    W25Q64_Write(PARAM_ADDR, buf, sizeof(buf));
}

void History_Add(const HistoryRecord *rec)
{
    history_buffer[history_head] = *rec;
    history_head = (history_head + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) {
        history_count++;
    }

    uint32_t addr = HISTORY_ADDR + 4 + (uint32_t)flash_record_count * 8;
    if (addr + 8 > HISTORY_ADDR + W25Q64_SECTOR_SIZE) {
        W25Q64_EraseSector(HISTORY_ADDR);
        flash_record_count = 0;
        addr = HISTORY_ADDR + 4;
    }
    W25Q64_Write(addr, (uint8_t *)rec, 8);
    flash_record_count++;
}

uint8_t History_GetCount(void)
{
    return history_count;
}

void History_GetRecord(uint8_t idx, HistoryRecord *rec)
{
    if (idx >= history_count) return;
    uint8_t pos;
    if (history_count < HISTORY_SIZE) {
        pos = idx;
    } else {
        pos = (history_head + idx) % HISTORY_SIZE;
    }
    *rec = history_buffer[pos];
}

void History_Clear(void)
{
    W25Q64_EraseSector(HISTORY_ADDR);
    flash_record_count = 0;
    history_count = 0;
    history_head = 0;
}
