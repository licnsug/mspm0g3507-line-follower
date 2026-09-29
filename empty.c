/*2025电赛寻迹*/









#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "Motor.h"
#include "Track.h"
#include "Encoder.h"
#include "PID.h"
#include "uart_vofa.h"
#include "LED.h"
#include "key.h"
#include "oled.h"

#define  BASE_SPEED    (20)   /* 基础巡航油门 0-100 */

int16_t SpeedM1, SpeedM2,gg;
uint8_t key_num,RunFlag,numm=0;       /* numm: 0=不限圈, 1~5=跑N圈 */
uint8_t cornerCnt=0, lapsDone=0;      /* 每4次弯道=1圈 */

int16_t err, diff, left, right;
static uint8_t track;

static int16_t lastDiff = 0;
static uint16_t lossCnt = 0;  /*丢线停车计时，每50ms+1，60=3s*/

PID_t Turn_PID= {                   //转向差速环
	.Kp=0.15,
	.Ki=0,
	.Kd=0.15,
	.OutMax=45,
	.OutMin=-45,
	.Target=0,                  /* 目标：黑线居中 */
};


int main(void)
{
    SYSCFG_DL_init();
    Motor_Init();
    Track_Init();
    //Encoder_Init();
    LED_Init();
    Key_Init();
    OLED_Init();
    OLED_ColorTurn(0);//0正常显示，1 反色显示
    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
    OLED_DisPlay_On();


    /* 启动 1ms 系统节拍 */
    DL_TimerG_startCounter(TIMER_G0_INST);
    NVIC_EnableIRQ(TIMER_G0_INST_INT_IRQN);

    OLED_ShowString(0,0,"track:",16,1);
    OLED_ShowChinese(0, 17, 11, 16, 1);  /* 圈 */
    OLED_ShowChinese(16,17, 12, 16, 1);  /* 数 */
    OLED_ShowString(33,17,":",16,1);

    while (1)
    {
        static uint8_t lastKey = 0;
        key_num = Key_Getnum();
        uint8_t keyEdge = (uint8_t)(key_num & ~lastKey);
        lastKey = key_num;
        if(RunFlag){LED_On();}else{LED_Off();}

        OLED_ShowBinary(60,0,track,16,1);
        //OLED_ShowSignedNum(40,33,err,3,16,1);
        OLED_ShowNum(40,17,numm,1,16,1);
        OLED_ShowNum(60,17,lapsDone,1,16,1);
        OLED_ShowNum(0,50,gg,3,16,1);

        OLED_Refresh();
        delay_ms(20);   /* 防止 SSD1306 刷新过快翻转 */
       

        if (keyEdge & 1)   /* 按键1：电机使能 */
        {
            if(RunFlag==0)
            {
                RunFlag=1;
                lapsDone=0;
                cornerCnt=0;
            }
            else
            {
                RunFlag=0;
                PID_Init(&Turn_PID);
                
            }
        }
        else if (keyEdge & 2)   /* 按键2：圈数设定 */
        {
            numm++;
            if(numm>=6)
            {
                numm=0;
            }

        }

        
        



    }
}

/* =========================== 1ms 系统节拍 ISR =========================== */

void TIMG0_IRQHandler(void)
{
    DL_TimerG_getPendingInterrupt(TIMER_G0_INST);

    
     Key_Tick();

    static uint16_t tick10ms = 0;
    static uint16_t tick50ms = 0;
    static uint16_t count3 = 0;
    static uint16_t lose;                /* 丢线检测标志位 */
    static uint8_t  cornerTrack = 0;     /* 10ms块捕捉的弯道特征 */

    tick10ms++;
    tick50ms++;
    count3++;
    gg++;

    /* 10ms 读传感器 + 丢线检测 + 弯道特征捕捉 */
    if (tick10ms >= 10) {
        tick10ms = 0;
        Track_Read();
        track = Track_GetState();
        lose = (track == 0xFF);

        if (track != 0xFF && track != 0x00) {
            uint8_t b7 = (track >> 7) & 1;
            uint8_t b0 = track & 1;
            if (b7 ^ b0) cornerTrack = track;
        }
    }

    /* 循迹控制：50ms PID + 输出 */
    if (tick50ms >= 50)
    {
        tick50ms = 0;
        err  = Track_Err();

        static int16_t lastErr   = 0;
        static uint8_t corner    = 0;    /* 0=无, 1=右弯, 2=左弯 */
        static uint8_t cornerOut = 0;    /* 出弯计数 */
        static uint8_t boostCnt  = 0;    /* 回正脉冲剩余次数 */
        static uint8_t boostSide = 0;    /* 1=boost左轮, 2=boost右轮 */

       if (RunFlag)
        {

            if (lose)                           /*如果丢线*/
            {
                lossCnt++;

                /* 弯道检测：10ms块捕捉的 cornerTrack bit7/bit0 直接判向 */
                if (!corner && cornerTrack) {
                    uint8_t b7 = (cornerTrack >> 7) & 1;
                    uint8_t b0 = cornerTrack & 1;
                    if      (b7 == 0 && b0 == 1) corner = 1;   /* 0xxxxxx1 → 左弯 */
                    else if (b7 == 1 && b0 == 0) corner = 2;   /* 1xxxxxx0 → 右弯 */
                    cornerCnt++;                                /* 入弯即计 */
                    if (cornerCnt >= 4) { lapsDone++; cornerCnt = 0; }
                    cornerTrack = 0;                            /* 消费 */
                }

                if (lossCnt > 80) {                     /* 真丢线 4s → 停车（最高优先级） */
                    Motor_Run(MOTOR_1, 0);
                    Motor_Run(MOTOR_2, 0);
                    RunFlag = 0;
                    PID_Init(&Turn_PID);
                    lossCnt = 0;
                    corner = 0;
                    cornerOut = 0;
                    boostCnt = 0;
                }
                else if (corner) {
                    /* 外侧轮满速，内侧轮低速 */
                    if (corner == 1)
                         { Motor_Run(MOTOR_1,  -10); Motor_Run(MOTOR_2, 40); }
                    else { Motor_Run(MOTOR_1, 40); Motor_Run(MOTOR_2,  -10); }
                }
                else {                                /* 普通丢线 → 记忆方向继续转 */
                    if (lastDiff > 0)
                         { left = 0; right = lastDiff; }
                    else if (lastDiff < 0)
                         { left = -lastDiff; right = 0; }
                    else if (lastErr > 0)
                         { left = 0; right = BASE_SPEED; }
                    else { left = BASE_SPEED; right = 0; }
                    Motor_Run(MOTOR_1, (int8_t)left);
                    Motor_Run(MOTOR_2, (int8_t)right);
                }
            }
            else
            {
                lossCnt = 0;
                lastErr = err;                          /* 记住偏差，弯道检测用 */

                /* 弯道结束：中线 0xE7 立刻退出，否则 250ms 超时 */
                if (corner) {
                    if (track == 0xE7) {
                        boostSide = corner;             /* 记住哪侧轮子需要回正 */
                        boostCnt  = 3;                  /* 回正脉冲 3 次(150ms) */
                        corner = 0; cornerOut = 0;
                    }
                    else if (track != 0x00) {
                        cornerOut++;
                        if (cornerOut > 5) { corner = 0; cornerOut = 0; }
                    } else { cornerOut = 0; }
                }

                /* ① 位置偏差 → PID */
                Turn_PID.Actual = (float)err;
                PID_Update(&Turn_PID);

                /* ② 差速值 */
                diff = (int16_t)Turn_PID.Out;
                lastDiff = diff;

                /* ③ 左右轮混控 */
                left  = BASE_SPEED + diff;
                right = BASE_SPEED - diff;

                /* ④ 限幅 */
                if (left  < 0)   left  = 0;
                if (left  > 100) left  = 100;
                if (right < 0)   right = 0;
                if (right > 100) right = 100;

                /* ⑤ 出弯回正脉冲：给弯道内侧轮额外 kick */
                if (boostCnt) {
                    if (boostSide == 1) left  += 25;   /* 右弯出 → 推左轮 */
                    else                right += 25;   /* 左弯出 → 推右轮 */
                    boostCnt--;
                }

                Motor_Run(MOTOR_1, (int8_t)left);
                Motor_Run(MOTOR_2, (int8_t)right);

                /* 圈数到且 track=11100111→ 自动停车 */
                if (numm && lapsDone >= numm && track == 0xE7) {
                    Motor_Run(MOTOR_1, 0);
                    Motor_Run(MOTOR_2, 0);
                    RunFlag = 0;
                    PID_Init(&Turn_PID);
                    lapsDone = 0;
                    cornerCnt = 0;
                }
            }

        }
        else
        {
            corner = 0;
            cornerOut = 0;
            boostCnt = 0;
            Motor_Run(MOTOR_1, 0);
            Motor_Run(MOTOR_2, 0);
        }
    }

    if (count3 >= 50) {
        count3 = 0;
        /* VOFA_printf("%d,%d\r\n",err,diff); */
       gg=0;
        
    }
}
