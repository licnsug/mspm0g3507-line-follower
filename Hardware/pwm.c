/**
 * @file  pwm.c
 * @brief 双路电机 PWM 控制实现
 *
 * 使用 TIMA0 外设，Edge-aligned Down Counting 模式。
 *
 * 占空比映射（down counting + INIT_VAL_LOW）：
 *   duty=0%   → ccValue=PWM_PERIOD  → 输出始终 LOW  → 电机停转
 *   duty=50%  → ccValue=PWM_PERIOD/2 → 输出 50% 方波
 *   duty=100% → ccValue=0           → 输出接近全 HIGH → 电机全速
 */

#include "ti_msp_dl_config.h"
#include "pwm.h"

/* -------------------------------------------------------------------------- */

void PWM_Start(void)
{
    /* SYSCFG_DL_PWM_0_init() 已在 SYSCFG_DL_init() 中调用，
     * 这里直接启动 TIMA0 计数器 */
    DL_TimerA_startCounter(PWM_0_INST);
}

/* -------------------------------------------------------------------------- */

void Motor_SetDuty(uint8_t motor, uint8_t duty)
{
    uint16_t ccValue;
    uint32_t ccIndex;

    /* 参数钳位 */
    if (duty > 100U) {
        duty = 100U;
    }

    /*
     * 占空比 → 比较值转换（Edge-aligned down counting, LACT=HIGH, CDACT=LOW）
     * CVAE=LDVAL: 启动时 counter = LOAD(7999)，递减过程不再经过 7999
     *   duty=0%  → CC=7998（1 tick HIGH → ≈0%）
     *   duty=50% → CC=4000（50% HIGH）
     *   duty=100%→ CC=0（≈100% HIGH）
     */
    if (duty == 0U) {
        ccValue = PWM_PERIOD - 2U;           /* 7998: 仅启动后第 1 个 tick 为 HIGH */
    } else if (duty >= 100U) {
        ccValue = 0U;                       /* CDACT 在 counter=0 触发 → ≈100% */
    } else {
        ccValue = (uint16_t)((uint32_t)PWM_PERIOD * (100U - duty) / 100U);
    }

    /* 选择通道 */
    if (motor == MOTOR_1) {
        ccIndex = GPIO_PWM_0_C0_IDX;        /* DL_TIMER_CC_0_INDEX */
    } else if (motor == MOTOR_2) {
        ccIndex = GPIO_PWM_0_C1_IDX;        /* DL_TIMER_CC_1_INDEX */
    } else {
        return;                             /* 无效电机编号 */
    }

    DL_TimerA_setCaptureCompareValue(PWM_0_INST, ccValue, ccIndex);
}




