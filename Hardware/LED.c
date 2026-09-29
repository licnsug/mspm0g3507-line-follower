/**
 * @file  LED.c
 * @brief 板载 LED 控制实现
 *
 * PB22 — LaunchPad LED2 Blue（J5 跳线帽需安装）
 * 高电平点亮，低电平熄灭。
 */

#include "ti_msp_dl_config.h"
#include "LED.h"

#define LED_PORT   GPIOB
#define LED_PIN    DL_GPIO_PIN_22

void LED_Init(void)
{
    DL_GPIO_initDigitalOutput(IOMUX_PINCM50);           /* PB22 IOMUX → GPIO 输出 */
    DL_GPIO_clearPins(LED_PORT, LED_PIN);               /* 初始熄灭 */
    DL_GPIO_enableOutput(LED_PORT, LED_PIN);
}

void LED_On(void)
{
    DL_GPIO_setPins(LED_PORT, LED_PIN);
}

void LED_Off(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_PIN);
}

void LED_Toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_PIN);
}
