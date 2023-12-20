#include "ws2812b.h"
#include "delay.h"	
#include "tim.h"


uint16_t Single_LED_Buffer[DATA_SIZE*LED_NUM+50];
u32 RGB_DATA;
u8 LEDDir;

extern TIM_HandleTypeDef htim16;
extern DMA_HandleTypeDef hdma_tim16_ch1;
extern u8 PowerState,LastPowerState;
extern u8 PWR_STATE;


void PWM_WS2812B_Init(void)
{
//	__HAL_TIM_DISABLE(&htim16);  							
	__HAL_TIM_ENABLE_DMA(&htim16, TIM_DMA_CC1);	
//	__HAL_DMA_DISABLE(&hdma_tim16_ch1);	
}

void WS2812B_Reset(void)
{
	__HAL_TIM_DISABLE(&htim16);  
	WS2812B_LOW;
	delay_ms(1);	
}

void PWM_WS2812B_Write_24Bits(uint16_t num,uint32_t GRB_Data)
{
  uint8_t i,j;
//	while(hdma_tim16_ch1.State != HAL_DMA_STATE_READY){};
  for(j = 0; j < num; j++)
  {
  	for(i = 0; i < DATA_SIZE; i++)
		{
			Single_LED_Buffer[j*DATA_SIZE+i] = ((GRB_Data << i) & 0x800000) ? T1H : T0H;
		}
  }
}


void PWM_WS2812B_Show(uint16_t num)
{
//	HAL_DMA_Start(&hdma_tim16_ch1,(uint32_t)Single_LED_Buffer,(uint32_t)&(TIM16->CCR1),num*DATA_SIZE);
  
	HAL_TIM_PWM_Start_DMA(&htim16,TIM_CHANNEL_1,(const uint32_t *)Single_LED_Buffer,num*DATA_SIZE+50);
//	__HAL_TIM_ENABLE(&htim16);  
//  while(__HAL_DMA_GET_FLAG(DMA1,DMA_FLAG_TC7) != SET){};
//	while(hdma_tim16_ch1.State != HAL_DMA_STATE_READY){};
		
//	HAL_TIM_PWM_Stop_DMA(&htim16,TIM_CHANNEL_1);
//	__HAL_DMA_DISABLE(&hdma_tim16_ch1);
//  __HAL_DMA_CLEAR_FLAG(DMA1,DMA_FLAG_TC7);
//  __HAL_TIM_DISABLE(&htim16); 
}


void PWM_WS2812B_Red(uint16_t num)
{
	PWM_WS2812B_Write_24Bits(num,0x00ff00);
	PWM_WS2812B_Show(num);
}

void PWM_WS2812B_Green(uint16_t num)
{
	PWM_WS2812B_Write_24Bits(num,0xff0000);
	PWM_WS2812B_Show(num);
}

void PWM_WS2812B_Blue(uint16_t num)
{
	PWM_WS2812B_Write_24Bits(num,0x0000ff);
	PWM_WS2812B_Show(num);
}

void PWM_WS2812B(uint16_t num,u32 RGB_data)
{
	PWM_WS2812B_Write_24Bits(num,RGB_data);
	PWM_WS2812B_Show(num);
}

void LEDUpdate(void)
{
	if(PWR_STATE==1)//电源打开
	{
		if(PowerState==1)
		{
			PWM_WS2812B_Write_24Bits(5,RGB_DATA);
		}
		else// if(LastPowerState!=PowerState)
		{
			PWM_WS2812B_Write_24Bits(5,0x0000ff);
		}
	}
	else
	{
		if(PowerState==1)
		{
			PWM_WS2812B_Write_24Bits(5,RGB_DATA);
		}
		else// if(LastPowerState!=PowerState)
		{
			PWM_WS2812B_Write_24Bits(5,0x000000);
		}
	}
	LastPowerState=PowerState;
}

