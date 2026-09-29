/**
 * @file  Encoder.h
 * @brief 双路编码器速度+方向测量接口
 *
 * 每路编码器使用 2 根信号线：
 *   A 相 → TIMGx 输入捕获（测速）
 *   B 相 → GPIO 输入（判方向：A 上升沿时 B=H→正转, B=L→反转）
 *
 * 硬件连接：
 *   编码器1 A相: PB6  (TIMG6 CCP0)    编码器1 B相: PB12
 *   编码器2 A相: PB15 (TIMG7 CCP0)    编码器2 B相: PB13
 *   VCC → 3.3V    GND → GND
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/* =========================== 用户可调参数 =========================== */

/** @brief 编码器每转脉冲数（电机端） */
#define ENCODER_PPR          (44U)
/** @brief 减速比（电机转数 : 输出轴转数） */
#define ENCODER_GEAR_RATIO   (30U)
/** @brief 输出轴每转脉冲数 = PPR × GEAR_RATIO */
#define ENCODER_TOTAL_PPR    (ENCODER_PPR * ENCODER_GEAR_RATIO)  /* 1320 */

/* =========================== 电机索引 =========================== */

#define ENCODER_MOTOR_1      (0U)
#define ENCODER_MOTOR_2      (1U)

/* =========================== 公开接口 =========================== */

/**
 * @brief 初始化编码器（启动输入捕获定时器、使能 NVIC 中断）
 */
void Encoder_Init(void);

/**
 * @brief 获取电机当前转速（带方向）
 * @param motor  ENCODER_MOTOR_1 或 ENCODER_MOTOR_2
 * @return      正值（×1000）= 正转转/秒，负值（×1000）= 反转转/秒，0 = 停止
 *
 * 示例：
 *   int16_t rps = Encoder_GetSpeed(ENCODER_MOTOR_1);
 *   // rps =  3000 → 电机1 正转 3.000 转/秒
 *   // rps = -1500 → 电机1 反转 1.500 转/秒
 */
int16_t Encoder_GetSpeed(uint8_t motor);

#endif /* ENCODER_H */
