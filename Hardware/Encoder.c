/**
 * @file  Encoder.c
 * @brief 双路编码器速度+方向测量实现
 *
 * 每路编码器 2 根信号线：
 *   A 相 → TIMGx 输入捕获 → 测量脉冲周期 → 计算 RPM
 *   B 相 → GPIO 输入      → A 相上升沿时读 B 电平 → 判断方向
 *
 * 方向判定逻辑（标准正交编码器）：
 *   A 相上升沿时 B=HIGH → 正转（+RPM）
 *   A 相上升沿时 B=LOW  → 反转（-RPM）
 *
 * 输出轴转速计算（PPR=44，减速比=30）：
 *   period_ticks = LOAD_VALUE - capture_value + 1
 *   输出总脉冲/转 = 44 × 30 = 1320
 *   输出转速(×1000 rps) = 1,250,000 × 1000 / (period_ticks × 1320)
 */

#include "ti_msp_dl_config.h"
#include "Encoder.h"

/* =========================== 定时器参数 =========================== */

/* 定时器时钟: 80MHz / 8 / 8 = 1.25 MHz (与 SysConfig 一致) */
#define ENCODER_TIMER_CLK_HZ    (1250000UL)

/* =========================== 内部状态 =========================== */

static volatile int16_t gEncoderSpeed[2];

/* =========================== ISR =========================== */

/**
 * @brief ENCODER_0 (TIMG6 + PB12 方向) 中断处理
 */
void TIMG6_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(ENCODER_0_INST)) {

    case DL_TIMERG_IIDX_CC0_DN: {
        uint16_t capture;
        uint16_t period;
        int16_t  sign;

        capture = DL_TimerG_getCaptureCompareValue(ENCODER_0_INST,
                    DL_TIMER_CC_0_INDEX);

        period = ENCODER_0_INST_LOAD_VALUE - capture + 1U;

        if (period > 0U) {
            /* 输出轴转速(×1000 转/秒) = clk * 1000 / (ticks * 总脉冲数) */
            uint32_t rps = ENCODER_TIMER_CLK_HZ * 1000UL
                         / ((uint32_t)period * ENCODER_TOTAL_PPR);

            /* B 相电平判断方向: HIGH=正转(+), LOW=反转(-) */
            sign = (DL_GPIO_readPins(GPIO_ENC_DIR_PORT,
                     GPIO_ENC_DIR_ENC_B1_PIN) != 0U) ? -1 : 1;

            gEncoderSpeed[ENCODER_MOTOR_1] = (int16_t)rps * sign;
        }

        /* Errata TIMER_ERR_01: 手动重载计数值 */
        DL_TimerG_setTimerCount(ENCODER_0_INST, ENCODER_0_INST_LOAD_VALUE);
        break;
    }

    case DL_TIMERG_IIDX_ZERO:
        gEncoderSpeed[ENCODER_MOTOR_1] = 0;
        break;

    default:
        break;
    }
}

/**
 * @brief ENCODER_1 (TIMG7 + PB13 方向) 中断处理
 */
void TIMG7_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(ENCODER_1_INST)) {

    case DL_TIMERG_IIDX_CC0_DN: {
        uint16_t capture;
        uint16_t period;
        int16_t  sign;

        capture = DL_TimerG_getCaptureCompareValue(ENCODER_1_INST,
                    DL_TIMER_CC_0_INDEX);

        period = ENCODER_1_INST_LOAD_VALUE - capture + 1U;

        if (period > 0U) {
            /* 输出轴转速(×1000 转/秒) = clk * 1000 / (ticks * 总脉冲数) */
            uint32_t rps = ENCODER_TIMER_CLK_HZ * 1000UL
                         / ((uint32_t)period * ENCODER_TOTAL_PPR);

            /* B 相电平判断方向 */
            sign = (DL_GPIO_readPins(GPIO_ENC2_DIR_PORT,
                     GPIO_ENC2_DIR_ENC_B2_PIN) != 0U) ? -1 : 1;

            gEncoderSpeed[ENCODER_MOTOR_2] = (int16_t)rps * sign;
        }

        DL_TimerG_setTimerCount(ENCODER_1_INST, ENCODER_1_INST_LOAD_VALUE);
        break;
    }

    case DL_TIMERG_IIDX_ZERO:
        gEncoderSpeed[ENCODER_MOTOR_2] = 0;
        break;

    default:
        break;
    }
}

/* =========================== 公开接口 =========================== */

void Encoder_Init(void)
{
    gEncoderSpeed[0] = 0;
    gEncoderSpeed[1] = 0;

    DL_TimerG_startCounter(ENCODER_0_INST);
    NVIC_EnableIRQ(ENCODER_0_INST_INT_IRQN);

    DL_TimerG_startCounter(ENCODER_1_INST);
    NVIC_EnableIRQ(ENCODER_1_INST_INT_IRQN);
}

int16_t Encoder_GetSpeed(uint8_t motor)
{
    if (motor == ENCODER_MOTOR_1) {
        return (int16_t)gEncoderSpeed[ENCODER_MOTOR_1];
    } else if (motor == ENCODER_MOTOR_2) {
        return (int16_t)gEncoderSpeed[ENCODER_MOTOR_2];
    }
    return 0;
}
