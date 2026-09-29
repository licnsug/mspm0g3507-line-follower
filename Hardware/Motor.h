#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "pwm.h"
#include "ti_msp_dl_config.h"


void Motor_Init();
void Motor_Run(uint8_t motor, int8_t duty);





#endif 
