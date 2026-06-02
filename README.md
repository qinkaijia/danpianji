# STM32F103C8T6 智能环境监测终端

基于 STM32F103C8T6（Blue Pill）的联网版智能环境监测系统，集成温湿度、超声波测距、人体红外感应、电机控制、Flash 存储、OLED 显示、蜂鸣器报警、编码器交互以及 ESP8266 WiFi 局域网 Web 服务器等功能。

## 功能特性

- **环境监测**：DHT11 温湿度 + HC-SR04 超声波测距 + PIR 人体红外感应
- **自动控制**：温度超标自动启动风扇（转速随温差线性增加），湿度/距离异常蜂鸣器报警
- **OLED 仪表盘**：128x64 实时显示温湿度、距离、人体检测状态、风扇转速和报警状态
- **菜单系统**：EC11 旋转编码器交互，支持阈值设置、手动风扇控制、历史记录浏览
- **掉电保存**：阈值参数通过 W25Q64 Flash 存储，重启后自动恢复
- **历史记录**：RAM 中保存最近 20 条环境数据，支持 OLED 翻页查看
- **WiFi Web 服务器**：ESP8266 连接 WiFi 热点，手机/电脑浏览器访问 IP 即可查看实时数据网页
- **LED 状态指示**：绿（系统运行）、黄（人体检测）、红（报警闪烁）
- **低功耗显示**：10 秒无操作自动熄屏，任意输入唤醒

## 硬件清单

| 模块 | 型号 | 用途 |
|------|------|------|
| MCU | STM32F103C8T6 | 主控芯片 |
| 显示屏 | SSD1306 128x64 OLED | 信息显示 |
| 温湿度 | DHT11 | 温度/湿度采集 |
| 超声波 | HC-SR04 | 距离测量 |
| 人体感应 | HC-SR501 PIR | 人体检测 |
| 电机驱动 | TB6612FNG | 风扇驱动 |
| 旋转编码器 | EC11 | 菜单交互 |
| Flash 存储 | W25Q64 | 参数掉电保存 |
| WiFi 模块 | ESP8266 | 局域网 Web 服务器 |
| 蜂鸣器 | 有源蜂鸣器 | 报警提示 |
| LED | 绿/黄/红 三色 | 状态指示 |

## 引脚分配

详见 [Pin.md](Pin.md)

## 项目结构

```
Jack/
├── Core/
│   ├── Inc/           # CubeMX 生成的头文件
│   ├── Src/           # CubeMX 生成的源文件
│   └── MY/            # 用户自定义驱动
│       ├── app.c/h        # 主应用状态机、菜单、自动控制
│       ├── tb6612.c/h     # 电机驱动
│       ├── oled.c/h       # OLED 显示
│       ├── pir.c/h        # PIR 人体感应
│       ├── buzzer.c/h     # 蜂鸣器
│       ├── encoder.c/h    # EC11 编码器
│       ├── w25q64.c/h     # W25Q64 Flash
│       ├── dht11.c/h      # DHT11 温湿度
│       ├── hcsr04.c/h     # HC-SR04 超声波
│       ├── esp8266.c/h    # ESP8266 WiFi
│       ├── led.c/h        # LED 指示灯
│       ├── key.c/h        # 独立按键
│       └── storage.c/h    # Flash 参数存储
├── Drivers/           # STM32 HAL 库和 CMSIS
├── cmake/             # CMake 工具链配置
├── CMakeLists.txt     # CMake 主配置
├── CMakePresets.json  # CMake 预设
├── Jack.ioc           # STM32CubeMX 配置文件
├── Pin.md             # 引脚分配文档
└── README.md          # 本文件
```

## 构建与烧录

### 环境要求

- ARM GCC 工具链（arm-none-eabi-gcc）
- CMake >= 3.22
- Ninja
- STM32CubeMX（可选，用于修改引脚/外设配置）

### 构建命令

```bash
# 配置 Debug 构建
cmake --preset Debug

# 编译
cmake --build --preset Debug
```

构建输出为 `build/Debug/Jack.elf`。

### 烧录

使用 VS Code + STM32CubeIDE 扩展，或 OpenOCD/ST-Link 工具烧录到目标板。

## 交互方式

| 操作 | 功能 |
|------|------|
| 编码器 **短按** | 确认 / 进入菜单 / 保存编辑 |
| 编码器 **旋转** | 切换菜单项 / 调整数值 / 翻页历史记录 |
| KEY_BACK **短按** | 返回上级 / 取消 |

## WiFi 配置

在 `Core/MY/esp8266.h` 中修改热点信息：

```c
#define ESP8266_SSID     "ningkai"
#define ESP8266_PASS     "0502080317"
```

上电后 ESP8266 自动连接 WiFi，开启 TCP Server（端口 80）。手机/电脑连接同一热点，浏览器访问 ESP8266 的 IP 地址即可查看实时环境数据网页。

## 默认阈值

| 参数 | 默认值 | 范围 |
|------|--------|------|
| 温度报警阈值 | 30°C | 20~50°C |
| 湿度报警阈值 | 80% | 50~100% |
| 距离报警阈值 | 10 cm | 5~100 cm |
| 记录间隔 | 5 分钟 | 1/5/10 分钟 |

首次上电时若 Flash 中无有效参数，系统自动写入默认值。

## 技术要点

- **HC-SR04 测距**：EXTI3 双边沿中断 + TIM4 自由运行计数器（1 tick = 1μs）测量回响时间
- **PIR 与 HC-SR04 EXTI 共享**：`HAL_GPIO_EXTI_Callback` 中通过 GPIO_Pin 分发到对应模块
- **DHT11 时序**：DWT 周期计数器实现 μs 级单总线时序
- **ESP8266 通信**：USART1 中断接收，AT 指令状态机管理 WiFi 连接和 HTTP 响应

## 许可证

本项目基于 STMicroelectronics HAL 库，CubeMX 生成的代码版权归 STMicroelectronics 所有。用户自定义驱动代码（`Core/MY/` 目录）可自由使用。
