#ifndef __OLED_H
#define __OLED_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* 类型别名 */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

/* 延时宏 */
#define delay_ms(ms)  delay_cycles((uint32_t)(ms) * 80000U)

/* 映射到 SysConfig 生成的硬件宏 */
#define OLED_PORT       GPIO_oled_PORT
#define OLED_RES_PIN    GPIO_oled_RES_PIN
#define OLED_DC_PIN     GPIO_oled_DC_PIN
#define OLED_CS_PIN     GPIO_oled_CS_PIN
#define SPI_OLED_INST   SPI_oled_INST

/* OLED 控制引脚 */
#define OLED_RES_Clr()  DL_GPIO_clearPins(OLED_PORT, OLED_RES_PIN)
#define OLED_RES_Set()  DL_GPIO_setPins(OLED_PORT, OLED_RES_PIN)

#define OLED_DC_Clr()   DL_GPIO_clearPins(OLED_PORT, OLED_DC_PIN)
#define OLED_DC_Set()   DL_GPIO_setPins(OLED_PORT, OLED_DC_PIN)

#define OLED_CS_Clr()   DL_GPIO_clearPins(OLED_PORT, OLED_CS_PIN)
#define OLED_CS_Set()   DL_GPIO_setPins(OLED_PORT, OLED_CS_PIN)

#define OLED_CMD  0	/* 写命令 */
#define OLED_DATA 1	/* 写数据 */

void OLED_ClearPoint(u8 x,u8 y);                                    /* 清除点 */
void OLED_ColorTurn(u8 i);                                          /* 反色切换 */
void OLED_DisplayTurn(u8 i);                                        /* 屏幕旋转180度 */
void OLED_WR_Byte(u8 dat,u8 mode);                                  /* 写数据/命令 */
void OLED_DisPlay_On(void);                                         /* 开启显示 */
void OLED_DisPlay_Off(void);                                        /* 关闭显示 */
void OLED_Refresh(void);                                            /* 刷新显存到屏幕 */
void OLED_Clear(void);                                              /* 清屏 */
void OLED_DrawPoint(u8 x,u8 y,u8 t);                                /* 画点 */
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);               /* 画线 */
void OLED_DrawCircle(u8 x,u8 y,u8 r);                               /* 画圆 */
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);             /* 显示字符 */
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);                   /* 显示6x8字符 */
void OLED_ShowString(u8 x,u8 y,const char *chr,u8 size1,u8 mode);  /* 显示字符串 */
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);      /* 显示数字 */
void OLED_ShowSignedNum(u8 x,u8 y,int16_t num,u8 len,u8 size1,u8 mode); /* 显示有符号数 */
void OLED_ShowBinary(u8 x,u8 y,u8 val,u8 size1,u8 mode);            /* 显示8位二进制 */
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);          /* 显示汉字 */
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);                   /* 滚动显示 */
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode); /* 显示图片 */
void OLED_Init(void);                                               /* OLED初始化 */

#endif

