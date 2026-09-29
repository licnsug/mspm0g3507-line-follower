/**
 * @file  pwm.h
 * @brief 双路电机 PWM 控制接口
 *
 * 基于 TIMA0 的 10kHz PWM 输出，两路独立占空比控制。
 * Channel 0 (CCP0) → PA0 → 电机 1
 * Channel 1 (CCP1) → PA1 → 电机 2
 */

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/* PWM 周期计数值（与 SysConfig 中 timerCount 一致） */
#define PWM_PERIOD      (40000U)   /* 必须与 SysConfig timerCount 一致 */

/* 电机编号 */
#define MOTOR_1         (1U)
#define MOTOR_2         (2U)

/**
 * @brief 初始化电机 PWM 并启动
 * @note  调用 SYSCFG_DL_PWM_0_init() 后启动 TIMA0 计数器
 */
void PWM_Start(void);

/**
 * @brief 设置电机占空比
 * @param motor  电机编号：MOTOR_1 或 MOTOR_2
 * @param duty   占空比 0~100（%），0=停转，100=全速
 */
void Motor_SetDuty(uint8_t motor, uint8_t duty);


#endif /* PWM_H */
