/**
 * @file  key.h
 * @brief 按键检测接口（3 按键，按下为低电平）
 *
 * 引脚: PB8 / PB21 / PB25，内部上拉，按下 → 0，松开 → 1。
 * Key_Tick() 需在 1ms 定时中断中调用，实现 20ms 消抖 + 下降沿检测。
 */

#ifndef KEY_H
#define KEY_H

#include <stdint.h>

void Key_Init(void);
void Key_Tick(void);           /* 每 1ms 调用一次 */
uint8_t Key_Getnum(void);      /* 返回触发按键号，无触发返回 0 */
uint8_t Key_Getstat(void);     /* 返回当前按下的按键号，无按下返回 0 */

#endif
