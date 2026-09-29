/**
 * @file  Track.h
 * @brief 感为 8 路灰度巡线传感器接口
 *
 * 协议：双线时钟串行（SCL + DAT），每帧读取 8 个 bit，
 *       每位对应一个传感器是否检测到黑线（1=检测到, 0=未检测到）。
 *
 * 硬件连接：
 *   PA13 (SCL) → 传感器 SCL
 *   PA12 (DAT) → 传感器 DAT
 *
 * 参考代码来源：感为八路灰度传感器手册示例（STM32 → MSPM0 移植）
 */

#ifndef TRACK_H
#define TRACK_H

#include <stdint.h>

/**
 * @brief 初始化传感器 GPIO（PA12=DAT 输入上拉, PA13=SCL 推挽输出）
 */
void Track_Init(void);

/**
 * @brief 读取传感器 8 路状态
 * @param arr  输出：8 位传感器状态（bit0=最右, bit7=最左，1=检测到黑线）
 */
void Track_Read(void);

/**
 * @brief 计算黑线位置偏差（供 PID 使用）
 *
 * 返回值已放大 10 倍（实际偏差 ×10），避免浮点运算。
 *   e.g. 3.5 → 35, -4.5 → -45
 *
 * @return  偏差值（×10）：负=线偏左需左转，正=线偏右需右转，0=居中
 */
int16_t Track_Err(void);

/**
 * @brief 获取滤波后的传感器状态字节
 * @return  8 位状态值
 */
uint8_t Track_GetState(void);
uint8_t Track_GetFiltered(void);

#endif /* TRACK_H */
