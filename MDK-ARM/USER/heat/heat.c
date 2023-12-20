#include "heat.h"
#include "tim.h"

u8 HeatPWMVal=0;

extern TIM_HandleTypeDef htim14;


void HeatPower(u8 state)
{
	state ? HAL_TIM_PWM_Start(&htim14,TIM_CHANNEL_1) : HAL_TIM_PWM_Stop(&htim14,TIM_CHANNEL_1);
}

void HeatPWMSet(u8 PWMVal)
{
	TIM14->CCR1=PWMVal;
}
























