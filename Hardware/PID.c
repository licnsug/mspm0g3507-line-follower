#include "ti_msp_dl_config.h"
#include "PID.h" 

/*PID 控制器的初始化函数，负责将一个 PID_t
结构体实例的所有内部状态清零，确保控制器从干净状态启动。*/

void PID_Init(PID_t *p)
{
	/*p->Target = 0;
	p->Actual = 0;
	p->Actual1 = 0;
	p->Out = 0;*/
	p->Error0 = 0;
	p->Error1 = 0;
	p->POut = 0;
	p->IOut = 0;
	p->DOut = 0;
}
/*
PID 控制器的更新函数，用于计算控制输出。该函数根据当前实际值和目标值，
计算比例、积分和微分项，最终输出控制信号。*/

void PID_Update(PID_t *p)
{
	/*误差传递*/
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;
	
	/*P项*/
	p->POut =  p->Kp * p->Error0;
	
	/*I项*/
	if (p->Ki != 0)
	{
		p->IOut +=  p->Ki * p->Error0;
	}
	else
	{
		p->IOut = 0;
	}
	
	/*积分限幅*/
	if (p->IOut > p->OutMax) {p->IOut = p->OutMax;}
	if (p->IOut < p->OutMin) {p->IOut = p->OutMin;}
	
	/*D项 + 微分先行*/
	p->DOut = - p->Kd * (p->Actual - p->Actual1);
	
	/*输出值*/
	p->Out = p->POut + p->IOut + p->DOut;
	
	/*输出偏移*/
	if (p->Out > 0) {p->Out += p->OutOffset;}
	if (p->Out < 0) {p->Out -= p->OutOffset;}
	
	/*输出限幅*/
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
	
	/*实际值传递*/
	p->Actual1 = p->Actual;
}


