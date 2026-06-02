#include "dht11.h"

#ifndef CoreDebug_DEMCR_TRCENA_Msk
#define CoreDebug_DEMCR_TRCENA_Msk (1UL << 24)
#endif
#ifndef DWT_CTRL_CYCCNTENA_Msk
#define DWT_CTRL_CYCCNTENA_Msk (1UL << 0)
#endif

static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

static uint8_t DHT11_WaitPinState(GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = timeout_us * (SystemCoreClock / 1000000);
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state) {
        if ((DWT->CYCCNT - start) > ticks) {
            return 1;
        }
    }
    return 0;
}

void DHT11_Init(void)
{
    DHT11_DWT_Init();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
}

int DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_DelayUs(30);

    if (DHT11_WaitPinState(GPIO_PIN_RESET, 100)) return -1;
    if (DHT11_WaitPinState(GPIO_PIN_SET,   100)) return -1;

    for (int i = 0; i < 40; i++) {
        if (DHT11_WaitPinState(GPIO_PIN_RESET, 100)) return -1;
        if (DHT11_WaitPinState(GPIO_PIN_SET,   100)) return -1;

        uint32_t start = DWT->CYCCNT;
        if (DHT11_WaitPinState(GPIO_PIN_RESET, 100)) return -1;
        uint32_t duration = DWT->CYCCNT - start;

        if (duration > 40 * (SystemCoreClock / 1000000)) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        return -1;
    }

    *humidity    = (float)data[0] + (float)data[1] / 10.0f;
    *temperature = (float)data[2] + (float)data[3] / 10.0f;

    if (data[2] & 0x80) {
        *temperature = -((float)(data[2] & 0x7F) + (float)data[3] / 10.0f);
    }

    return 0;
}
