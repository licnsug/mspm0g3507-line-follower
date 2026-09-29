/**
 * @file  uart_vofa.c
 * @brief VOFA+ 串口绘图与在线调参实现
 *
 * 支持 FireWater 协议波形显示 + 指令调参。
 * 通过 VOFA_printf 发送 CSV 格式数据，通过 vofa_set_data 接收调参指令。
 *
 * 更新记录:
 *   V0.0.1  2024-12-11  fu  初始版本，支持 FireWater 波形显示和收发指令
 */

#include "uart_vofa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PID.h"
#include "ti_msp_dl_config.h"

/* =========================== printf 串口重定向 =========================== */

int fputc(int ch, FILE *f)
{
    DL_UART_transmitDataBlocking(UART_bluetooth_INST, (uint8_t)ch);
    return ch;
}

/* =========================== 外部 PID 实例引用 =========================== */

extern PID_t Turn_PID;   /* 角度/转向环（定义于 empty.c） */
extern PID_t Speed_PID;   /* 速度/油门环（定义于 empty.c） */

/* VOFA 指令处理缓冲区 */
uint8_t g_ucVofaBuf[VOFA_DATAPACK_MAXLEN];

static float vofa_get_data(uint8_t, uint8_t);
static void  vofa_set_sram_data(uint8_t, float);
//static void  vofa_set_flash_data(float);

/* =========================== 公开接口 =========================== */



/**
 * @brief   通过串口向 VOFA+ 发送波形数据（FireWater 协议）
 * @param   _dataflag  当前打印的波形通道
 *
 * FireWater 协议格式:
 *   "<any>:ch0,ch1,ch2,...,chN\n"
 *   - any 和冒号可以为空，换行符(\n)不可省略
 *   - 多个通道用逗号分隔
 *
 *   示例:
 *     VOFA_printf("%.2f,%.2f,%.2f,%.2f\r\n",
 *                  target, current, kp, kd);
 *     VOFA_printf("channels: 1.386,0.977,-0.628,-0.942\n");
 */
void vofa_draw_graphical(uint8_t _dataflag)
{
    switch (_dataflag) {

    /* 速度环：Target / Actual / Out */
    case PID_LEFT_MOTOR:
        VOFA_printf("%.2f,%.2f,%.2f\r\n",
                    Speed_PID.Target,
                    Speed_PID.Actual,
                    Speed_PID.Out);
        break;

    /* 角度环：Target / Actual / Out */
    case PID_ANGLE:
        VOFA_printf("%.2f,%.2f,%.2f\r\n",
                    Turn_PID.Target,
                    Turn_PID.Actual,
                    Turn_PID.Out);
        break;

    default:
        break;
    }
}

/**
 * @brief   串口接收字节处理（放入串口接收中断/回调中）
 * @param   _rx_byte  接收到的单字节
 *
 * 自动解析 VOFA+ 调参指令格式:
 *   "指令号=值\n"
 *   例如 "P1=3.5\n" → 设置角度环 Kp = 3.5
 *        "D1=1.2\n" → 设置角度环 Kd = 1.2
 *        "T1=0.0\n" → 设置角度环 Target = 0.0
 *        "P2=25\n"  → 设置速度环 Kp = 25
 *        "I2=5\n"   → 设置速度环 Ki = 5
 */
void vofa_set_data(uint8_t _rx_byte)
{
    static uint8_t end_pos  = 0;  /* 记录帧尾位置 / 当前接收位置 */
    static uint8_t head_pos = 0;  /* 帧头位置 */

    /* 存入缓冲区 */
    g_ucVofaBuf[end_pos] = _rx_byte;

    if (_rx_byte == VOFA_DATAPACK_HEAD) {
        /* 找到帧头 '=' */
        head_pos = end_pos;
    }
    else if (_rx_byte == VOFA_DATAPACK_END
             && g_ucVofaBuf[head_pos] == VOFA_DATAPACK_HEAD) {
        /* 找到帧尾 '\n' 且之前有帧头 → 完整指令 */
        float vofaData = vofa_get_data(head_pos, end_pos);

        /* 根据指令号写入对应变量 */
        vofa_set_sram_data(head_pos, vofaData);

        /* 重置缓冲区 */
        end_pos  = 0;
        head_pos = 0;
        memset(g_ucVofaBuf, 0x00, VOFA_DATAPACK_MAXLEN);
    }

    /* 缓冲区溢出保护 */
    if (++end_pos > VOFA_DATAPACK_MAXLEN) {
        end_pos = 0;
        memset(g_ucVofaBuf, 0x00, VOFA_DATAPACK_MAXLEN);
    }
}

/* =========================== 内部函数 =========================== */

/**
 * @brief   从缓冲区提取指令数据部分，转为浮点数
 * @param   _head  帧头位置（'=' 所在位置）
 * @param   _end   帧尾位置（'\n' 所在位置）
 * @return  解析后的浮点数值
 *
 * 使用标准库 atof() 将字符串转为浮点数。
 * 数据部分为 _head+1 到 _end-1（即 "=" 和 "\n" 之间）。
 */
static float vofa_get_data(uint8_t _head, uint8_t _end)
{
    char _VofaData[_end - (_head + 1)];

    for (uint8_t i = 0; i < _end; i++) {
        _VofaData[i] = g_ucVofaBuf[i + _head + 1];
    }

    return atof(_VofaData);  /* 字符串转浮点 */
}

/**
 * @brief   将接收到的调参值写入对应的 SRAM 变量
 * @param   _head  帧头位置
 * @param   _data  浮点数值
 *
 * 指令格式:
 *   | 指令号(1-2byte) | 帧头('=',1byte) | 数据(n byte) | 帧尾('\n',1byte) |
 *
 *   例如:
 *     "P1=3.5\n"
 *     "V1=1.2\n"
 *
 * 指令号映射:
 *   角度环 [1]:
 *     P1 - g_tAnglePID.kp      比例系数
 *     D1 - g_tAnglePID.kd      微分系数
 *     T1 - g_tAnglePID.target  目标值
 *
 *   速度环 [2]:
 *     P2 - g_tLeftMotorPID.kp  比例系数
 *     I2 - g_tLeftMotorPID.ki  积分系数
 *     T2 - g_tLeftMotorPID.target 目标值
 *
 *   用户可在此扩展更多通道和指令。
 *   注意: 如果目标变量是 int 类型，记得做强制转换 (int)_data。
 */
static void vofa_set_sram_data(uint8_t _head, float _data)
{
    uint8_t id1 = g_ucVofaBuf[_head - 2];
    uint8_t id2 = g_ucVofaBuf[_head - 1];

    /* ---- 转向环 [1] ---- */
    if (id1 == 'P' && id2 == '1') {
        Turn_PID.Kp = _data;
    }
    else if (id1 == 'D' && id2 == '1') {
        Turn_PID.Kd = _data;
    }
    else if (id1 == 'I' && id2 == '1') {
        Turn_PID.Ki = _data;
    }
    else if (id1 == 'T' && id2 == '1') {
        Turn_PID.Target = _data;
    }
    

    
}

/**
 * @brief   将接收到的调参值写入 FLASH 存储（掉电不丢失）
 * @param   _data  浮点数值
 *
 * 用于持久化 PID 参数。需要在 FLASH 中开辟参数存储区，
 * 并调用片内 FLASH 写入 API 实现。
 * 当前为预留接口，暂未实现。
 */
/*static void vofa_set_flash_data(float _data)
{
    
}*/
