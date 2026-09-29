/**
 * @file  LED.h
 * @brief 板载 LED 控制接口
 *
 * PB22 — LaunchPad LED2 Blue（RGB 蓝色通道），高电平点亮。
 * 引脚: PB22, IOMUX_PINCM39, BP Pin 21 旁
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif
