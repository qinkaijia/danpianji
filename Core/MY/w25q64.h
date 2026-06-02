#ifndef __W25Q64_H
#define __W25Q64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define W25Q64_CS_PORT    GPIOB
#define W25Q64_CS_PIN     GPIO_PIN_12

#define W25X_WRITE_ENABLE       0x06
#define W25X_WRITE_DISABLE      0x04
#define W25X_READ_STATUS_REG1   0x05
#define W25X_READ_DATA          0x03
#define W25X_PAGE_PROGRAM       0x02
#define W25X_SECTOR_ERASE       0x20
#define W25X_CHIP_ERASE         0xC7
#define W25X_JEDEC_ID           0x9F

#define W25Q64_PAGE_SIZE        256
#define W25Q64_SECTOR_SIZE      4096

void W25Q64_Init(void);
uint32_t W25Q64_ReadID(void);
void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Q64_Write(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Q64_EraseSector(uint32_t addr);
void W25Q64_ChipErase(void);

#ifdef __cplusplus
}
#endif

#endif
