#include "ti_msp_dl_config.h"
#include "Motor.h"


void Motor_Init(void)
{
    
    /* 方向引脚拉低，H 桥刹车 */
    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_26 | DL_GPIO_PIN_27);
    DL_GPIO_clearPins(GPIOB, GPIO_dirB_PIN_4_PIN | GPIO_dirB_PIN_20_PIN);

    /* 启动定时器 */
    DL_TimerA_startCounter(PWM_0_INST);
}



/*电机序号，占空比（-100--100）*/
void Motor_Run(uint8_t motor, int8_t duty)
{
    /*电机1正反转控制*/
    if((motor == MOTOR_1)&&duty>=0)
    {
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_27 );
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_26);
        Motor_SetDuty(MOTOR_1,duty);
    }
    else if((motor == MOTOR_1)&&duty<0)
    {
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_26 );
        DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_27);
        Motor_SetDuty(MOTOR_1,-duty);
    }

    /*电机2正反转控制*/
    else if((motor == MOTOR_2)&&duty>=0)
    {
        DL_GPIO_setPins(GPIOB, GPIO_dirB_PIN_4_PIN );
        DL_GPIO_clearPins(GPIOB, GPIO_dirB_PIN_20_PIN);
        Motor_SetDuty(MOTOR_2,duty);
    }
    else if((motor == MOTOR_2)&&duty<0)
    {
        DL_GPIO_setPins(GPIOB, GPIO_dirB_PIN_20_PIN );
        DL_GPIO_clearPins(GPIOB, GPIO_dirB_PIN_4_PIN);
        Motor_SetDuty(MOTOR_2,-duty);
    }
    
   
}




