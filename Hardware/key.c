/**
 * @file  key.c
 * @brief 按键检测实现
 *
 * 2 按键: PB21(KEY1) / PB25(KEY2)
 * 内部上拉，按下 → 低电平，松开 → 高电平。
 * 20ms 消抖 + 下降沿触发。
 */

#include "ti_msp_dl_config.h"
#include "key.h"

/* 按键引脚定义 */
#define KEY1_PORT   GPIOB
#define KEY1_PIN    DL_GPIO_PIN_21
#define KEY1_IOMUX  IOMUX_PINCM49

#define KEY2_PORT   GPIOB
#define KEY2_PIN    DL_GPIO_PIN_25
#define KEY2_IOMUX  IOMUX_PINCM56

/* 消抖周期(ms) */
#define KEY_DEBOUNCE_MS   20U

static uint8_t Key_Num;   /* 触发按键号 */

/* =========================== 公开接口 =========================== */

void Key_Init(void)
{
    DL_GPIO_initDigitalInputFeatures(KEY1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY2_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    Key_Num = 0;
}

/**
 * @brief 每 1ms 调用，20ms 消抖 + 下降沿检测
 */
void Key_Tick(void)
{
    static uint16_t tick = 0;
    static uint8_t  prev = 0;
    static uint8_t  curr = 0;   /* static: 函数退出后保留上次采样值 */

    tick++;
    if (tick >= KEY_DEBOUNCE_MS) {
        tick = 0;
        prev = curr;                    /* 上个 20ms 窗口的稳定值 */
        curr = Key_Getstat();           /* 当前 20ms 窗口的稳定值 */

        /* 下降沿触发: 上周期没按，本周期按了 */
        if (prev == 0 && curr != 0) {
            Key_Num = curr;
        }
    }
}

/**
 * @brief 读取并清除触发按键号
 * @return 触发按键号(1/2)，读取后清零
 */
uint8_t Key_Getnum(void)
{
    uint8_t temp;

    if (Key_Num) {
        temp    = Key_Num;
        Key_Num = 0;
        return temp;
    }
    return 0;
}

/**
 * @brief 返回当前按下的按键号
 * @return 1=PB21, 2=PB25, 0=无按下
 */
uint8_t Key_Getstat(void)
{
    if (DL_GPIO_readPins(KEY1_PORT, KEY1_PIN) == 0) {
        return 1;
    }
    if (DL_GPIO_readPins(KEY2_PORT, KEY2_PIN) == 0) {
        return 2;
    }
    return 0;
}
