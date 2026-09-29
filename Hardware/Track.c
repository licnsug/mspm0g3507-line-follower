/**
 * @file  Track.c
 * @brief 感为 8 路灰度巡线传感器实现
 *
 * 通讯协议（双线时钟串行）：
 *   1. SCL 拉低
 *   2. 读取 DAT 电平（1 bit）
 *   3. SCL 拉高
 *   4. 延时 6µs
 *   5. 重复 8 次，得到 8 位传感器状态
 *
 * 8 位数据格式（bit7=最左传感器0, bit0=最右传感器7）：
 *   bit = 0 → 检测到黑线
 *   bit = 1 → 未检测到（白/灰背景）

    *硬件连接
    传感器引脚	MSPM0 引脚	功能
    DAT	PA12	数据输入（上拉）
    SCL	PA13	时钟输出（推挽）
    VCC	3.3V	电源
    GND	GND	地

 
 */

#include "ti_msp_dl_config.h"
#include "Track.h"

/* =========================== 引脚定义 =========================== */

#define TRACK_DAT_PORT    GPIOA
#define TRACK_DAT_PIN     DL_GPIO_PIN_12       /* PA12 — 传感器 DAT (输入) */
#define TRACK_SCL_PORT    GPIOA
#define TRACK_SCL_PIN     DL_GPIO_PIN_13       /* PA13 — 传感器 SCL (输出) */

/* SCL 半周期延时（6µs @ 80MHz = 480 cycles） */
#define TRACK_DELAY_CYCLES   (480U)

/* =========================== 内部状态 =========================== */

static uint8_t gTrackState = 0xFF;              /* EMA滤波后的传感器状态（初始全白） */
static uint8_t gTrackRaw;                       /* 原始传感器状态（弯道捕捉用） */

/* =========================== 公开接口 =========================== */

/**
 * @brief 初始化传感器引脚
 */
void Track_Init(void)
{
    DL_GPIO_initDigitalInputFeatures(
        IOMUX_PINCM34,                           /* PA12 IOMUX — DAT 输入 */
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(
        IOMUX_PINCM35);                         /* PA13 IOMUX — SCL 输出 */

    DL_GPIO_clearPins(TRACK_SCL_PORT, TRACK_SCL_PIN);
    DL_GPIO_enableOutput(TRACK_SCL_PORT, TRACK_SCL_PIN);

    gTrackState = 0U;
}

/**
 * @brief 读取 1 位数据（时钟下降沿触发传感器输出）
 */
static uint8_t Track_ReadBit(void)
{
    uint8_t bit;

    DL_GPIO_clearPins(TRACK_SCL_PORT, TRACK_SCL_PIN);   /* SCL = LOW  */
    bit = DL_GPIO_readPins(TRACK_DAT_PORT, TRACK_DAT_PIN) ? 1 : 0;  /* 位掩码归一化为 0/1 */
    DL_GPIO_setPins(TRACK_SCL_PORT, TRACK_SCL_PIN);     /* SCL = HIGH */
    delay_cycles(TRACK_DELAY_CYCLES);                    /* 延时 6µs  */

    return bit;
}

/**
 * @brief 读取 8 位传感器状态
 *
 * 8 位数据拼接方式（参考手册示例）：
 *   current = bit0*128 + bit1*64 + bit2*32 + bit3*16
 *           + bit4*8  + bit5*4  + bit6*2  + bit7*1
 *
 * 其中 bit0 是第一次读取的位（最左传感器 0），
 * bit7 是最后一次读取的位（最右传感器 7）。
 *
 * 滤波：当前值与上一次值取平均。
 */
void Track_Read()
{
    uint8_t bits[8];
    uint8_t n;
    uint8_t current;

    /* 读取 8 位 */
    for (n = 0; n < 8; n++) {
        bits[n] = Track_ReadBit();
    }

    /* 拼接为一个字节（MSB first: bit0=传感器0=最左） */
    current = bits[0] * 128 + bits[1] * 64 + bits[2] * 32 + bits[3] * 16
            + bits[4] * 8   + bits[5] * 4  + bits[6] * 2  + bits[7] * 1;

    gTrackRaw = current;                             /* 原始值 → 弯道捕捉 */
    gTrackState = (uint8_t)(((uint16_t)gTrackState * 7 + current) / 8); /* EMA α=1/8 */
    

    
}

/**
 * @brief 计算黑线位置偏差
 *
 * 查找表：将 8 位传感器组合模式映射为偏差值。
 *   正值 → 黑线偏右（需右转纠正）
 *   负值 → 黑线偏左（需左转纠正）
 *   0    → 居中
 *
 * 常见模式说明（以 1=传感器检测到黑线为例）：
 *   0xe7 = 11100111 → 中间 2 个传感器在黑线上 → Err=0
 *   0xf3 = 11110011 → 黑线略偏左          → Err=-3.5
 *   0xcf = 11001111 → 黑线偏右            → Err=+3.5
 */
int16_t Track_Err(void)
{
    switch (gTrackRaw) {
    case 0xe7: return    0;   /* 11100111  正中间             */
    case 0xcf: return   35;   /* 11001111  右侧小偏移    3.5  */
    case 0x9f: return   50;   /* 10011111  右侧中等偏移  5.0  */
    case 0x3f: return   60;   /* 00111111  右侧大偏移    6.0  */
    case 0xf3: return  -35;   /* 11110011  左侧小偏移   -3.5  */
    case 0xf9: return  -50;   /* 11111001  左侧中等偏移 -5.0  */
    case 0xfc: return  -60;   /* 11111100  左侧大偏移   -6.0  */
    case 0xef: return   20;   /* 11101111  右侧微偏移    2.0  */
    case 0xdf: return   30;   /* 11011111  右侧较小偏移  3.0  */
    case 0xbf: return   35;   /* 10111111  右侧较小偏移  3.5  */
    case 0x7f: return   70;   /* 01111111  右侧极限偏移  7.0  */
    case 0xf7: return  -20;   /* 11110111  左侧微偏移   -2.0  */
    case 0xfb: return  -30;   /* 11111011  左侧较小偏移 -3.0  */
    case 0xfd: return  -45;   /* 11111101  左侧较大偏移 -4.5  */
    case 0xfe: return  -70;   /* 11111110  左侧极限偏移 -7.0  */
    case 0x1f: return   80;   /* 00011111 右侧大偏移    8.0  */
    case 0x8f: return   50;   /* 10001111 右侧中等偏移  5.0  */
    case 0xc7: return   35;   /* 11000111 右侧小偏移    3.5  */
    case 0xe3: return  -35;   /* 11100011 左侧小偏移   -3.5  */
    case 0xf1: return  -50;   /* 11110001 左侧中等偏移 -5.0  */
    case 0xf8: return  -60;   /* 11111000 左侧大偏移   -6.0  */


    default:   return    0;   /*          未知状态，默认居中 */
    }
}

/**
 * @brief 获取原始传感器状态（弯道捕捉用）
 */
uint8_t Track_GetState(void)
{
    return gTrackRaw;
}

/**
 * @brief 获取 EMA 滤波后的传感器状态（OLED 显示用）
 */
uint8_t Track_GetFiltered(void)
{
    return gTrackState;
}
