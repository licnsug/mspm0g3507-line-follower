# MSPM0G3507 循迹小车 / Line-Following Car

![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)
![MCU](https://img.shields.io/badge/MCU-MSPM0G3507-red.svg)
![RTOS](https://img.shields.io/badge/RTOS-none-lightgrey.svg)
![Toolchain](https://img.shields.io/badge/toolchain-TI%20Clang-orange.svg)

**English** | [中文](#中文)

A line-following car written for the 2025 electronic design competition. It runs
bare metal (no RTOS) on an LP-MSPM0G3507 LaunchPad, built with Code Composer
Studio and the TI MSPM0 SDK.

---

## What it does

The car follows a black line with a sensor array, steering by driving its two
wheels at different speeds. A PID loop on the steering error produces the
differential; a second loop on encoder feedback keeps the base speed. The state
is shown on an OLED and can be streamed to a host for tuning.

* **Steering** — a PID on the line error (kp 0.15, ki 0, kd 0.15, output clamped
  to ±45) whose output becomes the left/right speed difference
* **Speed** — closed loop on quadrature encoder feedback, with a base cruise
  throttle
* **Lap counting** — four corners count as one lap; the count is shown on the
  display, and the run can be set to unlimited or 1-5 laps
* **Line loss protection** — if the line is lost for 3 s (60 ticks of the 50 ms
  control loop) the car stops instead of running away
* **UI** — SSD1306 OLED over SPI, plus a key to start and stop
* **Telemetry** — VOFA+ compatible UART stream and a 19200 baud Bluetooth link

## Hardware

Verified against the generated `ti_msp_dl_config.h`, so these are the pins the
firmware actually uses.

| Function | Peripheral | Pins |
| --- | --- | --- |
| Motor PWM, 2 channels | TIMA0 (80 MHz) | PA0 (CCP0), PA1 (CCP1) |
| Motor direction A | GPIOA | PA26, PA27 |
| Motor direction B | GPIOB | PB4, PB20 |
| Encoder 0 capture | TIMG6 CCP0 | PB6 |
| Encoder 1 capture | TIMG7 CCP0 | PB15 |
| Encoder B phase | GPIOB | PB12, PB13 |
| Control loop tick | TIMG0 (load 4999) | - |
| OLED control | GPIO | PB10 (RES), PB11 (DC), PB14 (CS) |
| OLED data | SPI1 | PB9 (SCLK), PB8 (PICO), PB7 (POCI) |
| Host telemetry | UART0 | PA10 (TX), PA11 (RX) |
| Bluetooth | UART1 ` 19200 | PA8 (TX), PA9 (RX) |
| External crystal | HFXT | PA5, PA6 |
| Debug | DEBUGSS | PA20 (SWCLK), PA19 (SWDIO) |

Target: MSPM0G3507, LQFP-64 (PM) package.

## Source layout

~~~text
empty.c            main loop, PID definitions, lap and display logic
empty.syscfg       SysConfig: every peripheral and pin assignment above
Hardware/
  Motor.c/.h       motor drive
  pwm.c/.h         PWM setup
  Encoder.c/.h     quadrature encoder reading
  Track.c/.h       line sensor handling
  PID.c/.h         PID controller
  oled.c/.h        SSD1306 driver over SPI
  oledfont.h       font tables
  bmp.h            bitmap tables
  key.c/.h         button input
  LED.c/.h         status LED
  uart_vofa.c/.h   VOFA+ telemetry
fix_syscfg_bug.sh  workaround for a SysConfig code generation bug (see below)
docs/              the original TI example readme this project started from
~~~

## Build

1. Install Code Composer Studio (Theia or classic) with the MSPM0 SDK
   (this project was generated with `mspm0_sdk` 2.10.00.04 and SysConfig
   1.27.1) and the TI Clang toolchain.
2. Import `empty_LP_MSPM0G3507_nortos_ticlang` as an existing CCS project.
3. Build and flash to an LP-MSPM0G3507.

The `Debug/` folder is build output and is not tracked.

## Known issue: SysConfig GPIO code generation

When the pin group `GPIO_dirA` is regenerated, SysConfig can emit
`DL_GPIO_PIN_0` for pins 26 and 27, which makes the direction pins alias PA0 and
silently breaks motor direction control. `fix_syscfg_bug.sh` patches the
generated header after each SysConfig run:

~~~bash
./fix_syscfg_bug.sh
~~~

Run it whenever the project is regenerated from `empty.syscfg`, then rebuild.

## Provenance

This project began as TI's "empty" DriverLib example for the LP-MSPM0G3507
LaunchPad; that original readme is kept under `docs/` for reference. The MSPM0
SDK, SysConfig and Code Composer Studio are TI's own tools and are covered by
TI's licenses, not by this repository's. `Hardware/oledfont.h` and
`Hardware/bmp.h` are font and bitmap data tables of the kind commonly published
with SSD1306 demo code, so check their origin before reuse.

## License

Released into the public domain under the [Unlicense](LICENSE). Use it for
anything, commercial or not, with no attribution required.

---

## 中文

**这是什么**：2025 电赛寻迹小车。裸机运行（无 RTOS），跑在 LP-MSPM0G3507 开发板上，用 Code Composer Studio 和 TI MSPM0 SDK 构建。

### 它做什么

小车用传感器阵列巡黑线，靠左右轮差速转向。转向误差经 PID 产生差速量，另一路用编码器反馈闭环维持基础速度；状态显示在 OLED 上，也可以串流到上位机调参。

* **转向**：对循迹偏差做 PID（Kp 0.15、Ki 0、Kd 0.15，输出限幅 ±45），输出直接作为左右轮差速
* **速度**：编码器正交解码闭环，另设基础巡航油门
* **圈数统计**：每 4 次弯道记 1 圈，显示在屏幕上，可设不限圈或 1–5 圈
* **丢线保护**：连续丢线 3 秒（50 ms 控制周期的 60 个 tick）自动停车，而不是冲出去
* **人机界面**：SPI 接 SSD1306 OLED，加按键启停
* **遥测**：VOFA+ 兼容的串口数据流，另有 19200 波特率的蓝牙链路

### 硬件配置

引脚是从实际生成的 `ti_msp_dl_config.h` 里核对出来的，也就是固件真正在用的那些：

| 功能 | 外设 | 引脚 |
| --- | --- | --- |
| 电机 PWM（两路） | TIMA0（80 MHz） | PA0（CCP0）、PA1（CCP1） |
| 电机方向 A | GPIOA | PA26、PA27 |
| 电机方向 B | GPIOB | PB4、PB20 |
| 编码器 0 捕获 | TIMG6 CCP0 | PB6 |
| 编码器 1 捕获 | TIMG7 CCP0 | PB15 |
| 编码器 B 相 | GPIOB | PB12、PB13 |
| 控制周期中断 | TIMG0（装载值 4999） | — |
| OLED 控制 | GPIO | PB10（RES）、PB11（DC）、PB14（CS） |
| OLED 数据 | SPI1 | PB9（SCLK）、PB8（PICO）、PB7（POCI） |
| 上位机遥测 | UART0 | PA10（TX）、PA11（RX） |
| 蓝牙 | UART1，19200 | PA8（TX）、PA9（RX） |
| 外部晶振 | HFXT | PA5、PA6 |
| 调试 | DEBUGSS | PA20（SWCLK）、PA19（SWDIO） |

目标芯片 MSPM0G3507，LQFP-64（PM）封装。

### 源码结构

`empty.c` 是主循环与 PID 定义；`empty.syscfg` 是 SysConfig 里全部外设与引脚配置；`Hardware/` 下按模块划分：电机、PWM、编码器、循迹、PID、OLED 与字库位图、按键、LED、VOFA+ 遥测；`fix_syscfg_bug.sh` 是 SysConfig 代码生成 bug 的修补脚本；`docs/` 放置项目起始时用的 TI 原版说明。

### 编译

装好 Code Composer Studio（Theia 或经典版）、MSPM0 SDK（本工程由 `mspm0_sdk` 2.10.00.04 与 SysConfig 1.27.1 生成）以及 TI Clang 工具链，然后把工程作为已有 CCS 工程导入，编译并烧录到 LP-MSPM0G3507。`Debug/` 是构建产物，不纳入版本管理。

### 已知问题：SysConfig 的 GPIO 代码生成

重新生成 `GPIO_dirA` 时，SysConfig 可能把第 26、27 脚输出成 `DL_GPIO_PIN_0`，于是方向引脚与 PA0 撞在一起，电机方向控制会悄悄失效。`fix_syscfg_bug.sh` 就是为此写的补丁：每次 SysConfig 重新生成之后运行一次，再重新编译。

~~~bash
./fix_syscfg_bug.sh
~~~

### 来源说明

本工程起始于 TI 为 LP-MSPM0G3507 提供的 "empty" DriverLib 例程，原版说明保留在 `docs/` 下。MSPM0 SDK、SysConfig、Code Composer Studio 都是 TI 的工具，适用 TI 自己的许可，不在本仓库的许可范围内。`Hardware/oledfont.h` 与 `Hardware/bmp.h` 是字库与位图数据表，这类表格通常来自 SSD1306 示例代码，复用前请确认其来源。

### 许可

以 [Unlicense](LICENSE) 释放到公有领域：随便用，商用也行，不需要署名。
