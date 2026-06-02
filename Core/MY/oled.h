#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "i2c.h"

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t color);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t color);
void OLED_ShowNumber(uint8_t x, uint8_t y, int32_t num, uint8_t color);

#ifdef __cplusplus
}
#endif

#endif
