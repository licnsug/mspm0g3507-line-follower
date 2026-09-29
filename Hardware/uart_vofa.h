/**
 * @file  uart_vofa.h
 * @brief VOFA+ 串口绘图调试接口
 *
 * 通过串口发送 FireWater 协议数据到 VOFA+ 上位机，
 * 实现 PID 参数实时波形显示和在线调参。
 *
 * 用法:
 *   vofa_draw_graphical(通道号);   // 放在 while/定时器 中，周期调用
 *   vofa_set_data(串口接收字节);    // 放在串口接收中断/回调中
 *
 * VOFA+ 协议说明:
 *   FireWater 格式: "ch0,ch1,ch2,...,chN\n"
 *   例如: printf("1.386,0.977,-0.628,-0.942\n");
 *
 *   调参指令格式: "指令号=值\n"
 *   例如: "P1=3.5\n"   设置角度环 Kp
 *         "D1=1.2\n"   设置角度环 Kd
 *         "T1=0.0\n"   设置角度环 Target
 *         "P2=25\n"    设置速度环 Kp
 *         "I2=5\n"     设置速度环 Ki
 接线：
USB-TTL 模块          LaunchPad
   USB-TTL TX  →  PA9  (RX)   BP Pin 3
USB-TTL RX  →  PA8  (TX)   BP Pin 4


 */

#ifndef __UART_VOFA_H__
#define __UART_VOFA_H__

/* 防止头文件重复定义基础类型 */
typedef unsigned char   uint8_t;
typedef uint8_t         u8;

/* =========================== VOFA 显示设置 =========================== */

/*
 * 当前使用的数据引擎:
 *   FireWater: CSV 字符串格式，直观简洁，适合低频调试（<200Hz）
 *   JustFloat: 小端浮点字节流，适合高频传输（>200Hz）
 *   RawData:   原始字节，无额外开销
 */
#define VOFA_DATA_FORMAT "FireWater"

/*
 * 将 VOFA_printf 映射到目标平台的 printf 函数
 * （需确保串口 printf 重定向已配置）
 */
#define VOFA_printf     printf

/*
 * 波形打印通道选择（对应 vofa_draw_graphical 中的 switch case）
 * 用户可按需扩展
 */
#define PID_LEFT_MOTOR  0       /* 左电机速度环 */
#define PID_RIGHT_MOTOR 1       /* 右电机速度环 */
#define PID_TURN        2       /* 转向环 */
#define PID_ANGLE       3       /* 角度环 */

/* =========================== VOFA 调参指令格式 =========================== */

/*
 * VOFA+ 指令格式:
 *
 *   | 指令号(1-2byte) | 帧头('=',1byte) | 数据(n byte) | 帧尾('\n',1byte) |
 *
 *   例如:
 *     "P1=3.5\n"   设置通道1的P参数
 *     "V2=1.2\n"   设置通道2的值
 *
 *   这里，指令号: "P1/V2"，帧头: "="，帧尾: "\n"，数据: 浮点数字
 */
#define VOFA_DATAPACK_HEAD      '='     /* VOFA 指令帧头 */
#define VOFA_DATAPACK_END       '\n'    /* VOFA 指令帧尾 */
#define VOFA_DATAPACK_MAXLEN    200     /* VOFA 指令最大长度 */

/* =========================== 公开接口 =========================== */

/**
 * @brief   通过串口向 VOFA+ 发送波形数据
 * @param   _dataflag  当前打印的波形通道（参见 PID_LEFT_MOTOR 等宏）
 *
 * 调用示例:
 *   vofa_draw_graphical(PID_ANGLE);  // 发送角度环 PID 波形数据
 */
void vofa_draw_graphical(uint8_t _dataflag);

/**
 * @brief   串口接收字节处理（放入串口接收中断/回调）
 * @param   _rx_byte  串口接收到的单字节
 *
 * 自动解析 VOFA+ 调参指令并执行对应操作。
 * 例如收到 "P1=3.5\n" 会设置 g_tAnglePID.kp = 3.5
 */
void vofa_set_data(uint8_t _rx_byte);

#endif /* __UART_VOFA_H__ */
