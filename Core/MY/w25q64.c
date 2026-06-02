#include "w25q64.h"
#include "spi.h"

static void W25Q64_CS_Low(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_RESET);
}

static void W25Q64_CS_High(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_SET);
}

static uint8_t W25Q64_SPI_ReadWriteByte(uint8_t byte)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &rx, 1, 100);
    return rx;
}

static void W25Q64_WaitBusy(void)
{
    uint8_t status;
    do {
        W25Q64_CS_Low();
        W25Q64_SPI_ReadWriteByte(W25X_READ_STATUS_REG1);
        status = W25Q64_SPI_ReadWriteByte(0xFF);
        W25Q64_CS_High();
    } while (status & 0x01);
}

static void W25Q64_WriteEnable(void)
{
    W25Q64_CS_Low();
    W25Q64_SPI_ReadWriteByte(W25X_WRITE_ENABLE);
    W25Q64_CS_High();
}

void W25Q64_Init(void)
{
    W25Q64_CS_High();
}

uint32_t W25Q64_ReadID(void)
{
    uint8_t tx[4] = {W25X_JEDEC_ID, 0xFF, 0xFF, 0xFF};
    uint8_t rx[4];
    W25Q64_CS_Low();
    HAL_SPI_TransmitReceive(&hspi2, tx, rx, 4, 100);
    W25Q64_CS_High();
    return ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | rx[3];
}

void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    W25Q64_CS_Low();
    W25Q64_SPI_ReadWriteByte(W25X_READ_DATA);
    W25Q64_SPI_ReadWriteByte((addr >> 16) & 0xFF);
    W25Q64_SPI_ReadWriteByte((addr >> 8) & 0xFF);
    W25Q64_SPI_ReadWriteByte(addr & 0xFF);
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = W25Q64_SPI_ReadWriteByte(0xFF);
    }
    W25Q64_CS_High();
}

void W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint32_t len)
{
    if (len > W25Q64_PAGE_SIZE) len = W25Q64_PAGE_SIZE;
    W25Q64_WriteEnable();
    W25Q64_CS_Low();
    W25Q64_SPI_ReadWriteByte(W25X_PAGE_PROGRAM);
    W25Q64_SPI_ReadWriteByte((addr >> 16) & 0xFF);
    W25Q64_SPI_ReadWriteByte((addr >> 8) & 0xFF);
    W25Q64_SPI_ReadWriteByte(addr & 0xFF);
    for (uint32_t i = 0; i < len; i++) {
        W25Q64_SPI_ReadWriteByte(buf[i]);
    }
    W25Q64_CS_High();
    W25Q64_WaitBusy();
}

void W25Q64_Write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t page_offset = addr % W25Q64_PAGE_SIZE;
    uint32_t write_len;

    while (len > 0) {
        write_len = W25Q64_PAGE_SIZE - page_offset;
        if (write_len > len) write_len = len;
        W25Q64_PageProgram(addr, buf, write_len);
        addr += write_len;
        buf += write_len;
        len -= write_len;
        page_offset = 0;
    }
}

void W25Q64_EraseSector(uint32_t addr)
{
    W25Q64_WriteEnable();
    W25Q64_CS_Low();
    W25Q64_SPI_ReadWriteByte(W25X_SECTOR_ERASE);
    W25Q64_SPI_ReadWriteByte((addr >> 16) & 0xFF);
    W25Q64_SPI_ReadWriteByte((addr >> 8) & 0xFF);
    W25Q64_SPI_ReadWriteByte(addr & 0xFF);
    W25Q64_CS_High();
    W25Q64_WaitBusy();
}

void W25Q64_ChipErase(void)
{
    W25Q64_WriteEnable();
    W25Q64_CS_Low();
    W25Q64_SPI_ReadWriteByte(W25X_CHIP_ERASE);
    W25Q64_CS_High();
    W25Q64_WaitBusy();
}
