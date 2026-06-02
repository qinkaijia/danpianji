# Pin Assignment Record

> 本文档记录本项目各模块的引脚分配与接法，每次新增模块后更新。

---

## 系统预留引脚

| 功能 | 引脚 | 说明 |
|------|------|------|
| SWDIO | PA13 | 调试接口，不可占用 |
| SWCLK | PA14 | 调试接口，不可占用 |
| HSE 晶振 | PD0 / PD1 | 8 MHz 外部晶振 |
| LSE 晶振 | PC14 / PC15 | 32.768 kHz 外部晶振 |
| 板载 LED | PC13 | 低电平点亮，510Ω 限流 |
| BOOT0 | — | 跳线配置，接地为正常启动 |
| BOOT1 (PB2) | PB2 | 100K 上拉至 VCC，建议保留 |

---

## 模块引脚分配

### 1. TB6612 直流电机驱动

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| PWMA | PB0 | TIM3_CH3 (PWM) | TB6612 驱动芯片 PWMA 引脚 |
| AIN1 | PA7 | GPIO_Output | TB6612 驱动芯片 AIN1 引脚 |
| AIN2 | PB1 | GPIO_Output | TB6612 驱动芯片 AIN2 引脚 |

**CubeMX 配置**：
- TIM3 → PWM Generation CH3 on PB0
- PA7 → GPIO_Output (push-pull)
- PB1 → GPIO_Output (push-pull)

**驱动文件**：`Core/MY/tb6612.c`, `Core/MY/tb6612.h`

**API**：
- `TB6612_Init()` — 启动 PWM，初始停止
- `TB6612_SetSpeed(int16_t speed)` — 速度 -100 ~ 100（百分比）
- `TB6612_Stop()` — 待机（AIN1=0, AIN2=0）
- `TB6612_Brake()` — 刹车（AIN1=1, AIN2=1）

---

### 2. SSD1306 OLED 显示屏 (I2C)

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| SCL | PB6 | I2C1_SCL | OLED SCK/SCL 引脚 |
| SDA | PB7 | I2C1_SDA | OLED SDA 引脚 |

**CubeMX 配置**：
- I2C1 → I2C 模式，PB6/PB7 自动分配
- 速率：Standard Mode (100 kHz)

**驱动文件**：`Core/MY/oled.c`, `Core/MY/oled.h`

**API**：
- `OLED_Init()` — 初始化 SSD1306，清屏
- `OLED_Clear()` — 清空显存
- `OLED_Refresh()` — 刷新显存到屏幕
- `OLED_ShowString(x, y, str, color)` — 显示字符串
- `OLED_ShowNumber(x, y, num, color)` — 显示整数
- `OLED_DrawPixel/Line/Rectangle(x, ...)` — 基本绘图

### 3. HC-SR501 人体红外感应模块 (PIR)

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| OUT | PB5 | GPIO_EXTI5 | HC-SR501 输出引脚 |
| VCC | — | — | HC-SR501 VCC（3.3V 或 5V） |
| GND | — | — | HC-SR501 GND |

**CubeMX 配置**：
- PB5 → `GPIO_EXTI5`
- GPIO mode: `External Interrupt Mode with Rising edge trigger detection`
- GPIO Pull-up/Pull-down: `Pull-down`
- NVIC: 使能 `EXTI line[9:5] interrupts`

**驱动文件**：`Core/MY/pir.c`, `Core/MY/pir.h`

**API**：
- `PIR_Init()` — 初始化状态标志
- `PIR_GetState()` — 读取当前引脚电平（0/1）
- `PIR_IsMotionDetected()` — 检测是否触发过人（中断标志）
- `PIR_ClearMotionFlag()` — 清除触发标志

### 4. 有源蜂鸣器（低电平触发）

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| 控制端 | PB4 | GPIO_Output | 蜂鸣器控制引脚（低电平响） |
| VCC | — | — | 蜂鸣器 VCC（3.3V） |
| GND | — | — | 蜂鸣器 GND |

**CubeMX 配置**：
- PB4 → `GPIO_Output`
- GPIO output level: `High`（初始不响）
- GPIO mode: `Output Push Pull`

> 选用 PB4 理由：当前项目使用 SWD（Serial Wire），JTAG 已禁用，PB4 已释放为普通 GPIO；无 USART/I2C/SPI/CAN/USB 等重要复用功能。

**驱动文件**：`Core/MY/buzzer.c`, `Core/MY/buzzer.h`

**API**：
- `Buzzer_Init()` — 初始化，默认关闭
- `Buzzer_On()` — 低电平输出，蜂鸣器响
- `Buzzer_Off()` — 高电平输出，蜂鸣器停
- `Buzzer_Toggle()` — 翻转状态
- `Buzzer_Beep(ms)` — 响指定毫秒后关闭

### 5. EC11 旋转编码器（TIM2 硬件编码器模式）

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| CLK (A相) | PA0 | TIM2_CH1 | EC11 CLK 引脚 |
| DT (B相) | PA1 | TIM2_CH2 | EC11 DT 引脚 |
| SW (按键) | PB3 | GPIO_Input | EC11 SW 引脚 |
| VCC | — | — | EC11 VCC（3.3V） |
| GND | — | — | EC11 GND |

**CubeMX 配置**：
- PA0 → `TIM2_CH1`
- PA1 → `TIM2_CH2`
- PB3 → `GPIO_Input`，Pull-up
- TIM2 → `Combined Channels: Encoder Mode`
- Encoder Mode: `TI1 and TI2`
- Counter Period: `65535`
- NVIC: 使能 `TIM2 global interrupt`

> 选用 TIM2 硬件编码器模式理由：STM32 内部硬件正交解码，CPU 零开销，精度高（每格 4 个脉冲）。PA0/PA1 是 TIM2 默认通道，无需重映射。

**驱动文件**：`Core/MY/encoder.c`, `Core/MY/encoder.h`

**API**：
- `Encoder_Init()` — 启动 TIM2 编码器
- `Encoder_GetCount()` — 读取当前计数值
- `Encoder_GetDelta()` — 获取相对上一次的变化量
- `Encoder_ClearCount()` — 清零计数器
- `Encoder_IsPressed()` — 检测按键按下（带消抖）

### 6. W25Q64 SPI Flash 存储模块

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| SCK | PB13 | SPI2_SCK | W25Q64 CLK 引脚 |
| MISO | PB14 | SPI2_MISO | W25Q64 DO 引脚 |
| MOSI | PB15 | SPI2_MOSI | W25Q64 DI 引脚 |
| CS | PB12 | GPIO_Output | W25Q64 CS 引脚（软件片选） |
| WP | — | — | 接 VCC（3.3V，禁用写保护） |
| HOLD | — | — | 接 VCC（3.3V，禁用保持） |
| VCC | — | — | W25Q64 VCC（3.3V） |
| GND | — | — | W25Q64 GND |

**CubeMX 配置**：
- PB12 → `GPIO_Output`（软件 CS）
- PB13 → `SPI2_SCK`
- PB14 → `SPI2_MISO`
- PB15 → `SPI2_MOSI`
- SPI2 → `Full-Duplex Master`
- Hardware NSS Signal: `Disable`
- Prescaler: `Baud Rate = 9 MBits/s`
- Clock Polarity: `Low`
- Clock Phase: `1 Edge`

> 选用 SPI2 理由：PB12~PB15 是完整的硬件 SPI2 接口，不占用 USART/I2C/USB 等通信引脚；CS 用软件控制，方便扩展多从机。

**驱动文件**：`Core/MY/w25q64.c`, `Core/MY/w25q64.h`

**API**：
- `W25Q64_Init()` — 初始化 CS
- `W25Q64_ReadID()` — 读取 JEDEC ID，返回 24 位 ID
- `W25Q64_Read(addr, buf, len)` — 从指定地址读取数据
- `W25Q64_Write(addr, buf, len)` — 写入数据（自动处理页边界）
- `W25Q64_EraseSector(addr)` — 擦除 4KB Sector
- `W25Q64_ChipErase()` — 整片擦除

### 7. DHT11 数字温湿度传感器

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| DATA | PA8 | GPIO_Output (Open Drain) | DHT11 数据引脚 |
| VCC | — | — | DHT11 VCC（3.3V 或 5V） |
| GND | — | — | DHT11 GND |

**CubeMX 配置**：
- PA8 → `GPIO_Output`
- GPIO mode: `Output Open Drain`
- GPIO Pull-up/Pull-down: `Pull-up`

> DHT11 模块通常已板载 10K 上拉电阻，CubeMX 中同时开启内部上拉以提高总线可靠性。

**驱动文件**：`Core/MY/dht11.c`, `Core/MY/dht11.h`

**API**：
- `DHT11_Init()` — 初始化 GPIO 和 DWT 周期计数器
- `DHT11_Read(float *temperature, float *humidity)` — 读取温湿度，成功返回 0，失败返回 -1

> **注意**：DHT11 两次读取间隔必须 ≥ 1 秒，否则返回错误。

### 8. HC-SR04 超声波测距模块

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| TRIG | PA2 | GPIO_Output | HC-SR04 TRIG 引脚 |
| ECHO | PA3 | GPIO_Input | HC-SR04 ECHO 引脚 |
| VCC | — | — | HC-SR04 VCC（3.3V） |
| GND | — | — | HC-SR04 GND |

**CubeMX 配置**：
- PA2 → `GPIO_Output`
  - GPIO output level: `Low`
  - GPIO mode: `Output Push Pull`
- PA3 → `GPIO_Input`
  - GPIO Pull-up/Pull-down: `Pull-down`

> ECHO 在 3.3V 供电下输出 3.3V 电平，可直接接 PA3，无需限流电阻。

**驱动文件**：`Core/MY/hcsr04.c`, `Core/MY/hcsr04.h`

**API**：
- `HCSR04_Init()` — 初始化 GPIO 和 DWT
- `HCSR04_ReadCm()` — 读取距离（厘米），成功返回 2~400，失败返回 -1

> **注意**：两次测量间隔建议 ≥ 60ms，避免超声波回波干扰。

### 9. ESP8266 WiFi 模块

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| TX | PA10 | USART1_RX | ESP8266 UART TX → STM32 RX |
| RX | PA9 | USART1_TX | STM32 TX → ESP8266 UART RX（建议串 1KΩ 限流电阻） |
| VCC | — | — | 3.3V（必须 3.3V，禁止 5V） |
| GND | — | — | GND |
| EN | — | — | 接 3.3V（使能） |
| GPIO0 | — | — | 接 3.3V（正常启动模式） |

**CubeMX 配置**：
- USART1 → Baud Rate: 115200，Word Length: 8 Bits，Stop Bits: 1
- PA9 → `USART1_TX` (Alternate Function Push Pull)
- PA10 → `USART1_RX` (Input, No Pull-up/Pull-down)
- NVIC: 使能 `USART1 global interrupt`

**驱动文件**：`Core/MY/esp8266.c`, `Core/MY/esp8266.h`

**API**：
- `ESP8266_Init()` — 初始化 USART1 中断接收，启动 AT 指令状态机
- `ESP8266_Task()` — 主循环调用，处理 WiFi 连接和 HTTP 请求
- `ESP8266_GetState()` — 获取当前网络状态

> 手机/电脑连同一 WiFi，浏览器访问 ESP8266 的 IP 地址即可查看实时环境数据网页。

### 10. LED 指示灯（绿 / 黄 / 红）

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| 绿色 LED | PA4 | GPIO_Output | 绿色指示灯（系统正常运行，常亮） |
| 黄色 LED | PA5 | GPIO_Output | 黄色指示灯（PIR 检测到人体时亮 1 秒） |
| 红色 LED | PA6 | GPIO_Output | 红色指示灯（报警状态时闪烁） |

**CubeMX 配置**：
- PA4/PA5/PA6 → `GPIO_Output`
- GPIO mode: `Output Push Pull`
- GPIO output level: `Low`（初始熄灭）

**驱动文件**：`Core/MY/led.c`, `Core/MY/led.h`

**API**：
- `LED_Init()` — 初始化，全部熄灭
- `LED_Green(state)` — 绿色灯控制（1=亮，0=灭）
- `LED_Yellow(state)` — 黄色灯控制
- `LED_Red(state)` — 红色灯控制
- `LED_RedToggle()` — 红色灯翻转

### 11. 独立按键（返回/取消）

| 信号 | STM32 引脚 | 外设功能 | 接法 |
|------|-----------|---------|------|
| KEY | PB8 | GPIO_Input | 独立按键，按下为低电平 |

**CubeMX 配置**：
- PB8 → `GPIO_Input`
- GPIO Pull-up/Pull-down: `Pull-up`

> PB8 为普通 GPIO 引脚，无 BOOT 功能，无重要外设复用冲突，适合作为独立按键输入。

**驱动文件**：`Core/MY/key.c`, `Core/MY/key.h`

**API**：
- `Key_Init()` — 初始化（无操作，CubeMX 已配置 GPIO）
- `Key_IsPressed()` — 检测按键按下（带 20ms 消抖，阻塞等待释放）

---

## 引脚占用总览

| 引脚 | 当前用途 | 可复用功能（冲突） |
|------|---------|-------------------|
| PA7 | TB6612 AIN1 | TIM3_CH2, SPI1_MOSI, ADC1_IN7 |
| PB0 | TB6612 PWMA | TIM3_CH3, ADC1_IN8 |
| PB1 | TB6612 AIN2 | TIM3_CH4, ADC1_IN9 |
| PB6 | OLED SCL | I2C1_SCL, TIM4_CH1, USART1_TX remap |
| PB7 | OLED SDA | I2C1_SDA, TIM4_CH2, USART1_RX remap |
| PB5 | PIR OUT | EXTI5, TIM3_CH2 remap, SPI1_MOSI remap |
| PB4 | Buzzer | GPIO_Output（JTAG 已禁用，纯 GPIO） |
| PA0 | EC11 CLK | TIM2_CH1（编码器模式） |
| PA1 | EC11 DT | TIM2_CH2（编码器模式） |
| PB3 | EC11 SW | GPIO_Input（JTAG 已禁用，纯 GPIO） |
| PB12 | W25Q64 CS | GPIO_Output（SPI2_NSS，软件控制） |
| PB13 | W25Q64 SCK | SPI2_SCK |
| PB14 | W25Q64 MISO | SPI2_MISO |
| PB15 | W25Q64 MOSI | SPI2_MOSI |
| PA8  | DHT11 DATA | GPIO_Output (Open Drain) |
| PA2  | HCSR04 TRIG | GPIO_Output |
| PA3  | HCSR04 ECHO | GPIO_Input |
| PA9  | ESP8266 TX | USART1_TX |
| PA10 | ESP8266 RX | USART1_RX |
| PA4  | LED 绿色 | GPIO_Output |
| PA5  | LED 黄色 | GPIO_Output |
| PA6  | LED 红色 | GPIO_Output |
| PB8  | KEY_BACK | GPIO_Input |
| PA13 | SWDIO | 不可占用 |
| PA14 | SWCLK | 不可占用 |
| PC13 | 板载 LED | 可用但建议保留 |
| PC14/PC15 | LSE 晶振 | 不可用 |
| PD0/PD1 | HSE 晶振 | 不可用 |

---

## 空闲引脚（可用）

**PA 口**：PA11, PA12, PA15

**PB 口**：PB2, PB9, PB10, PB11

**PC 口**：PC13（板载 LED，可用）
