#ifndef _WS2812B_H
#define _WS2812B_H
#include <sys.h>	  


#define WS2812B_HIGH   HAL_GPIO_WritePin(WS2812B_GPIO_Port,WS2812B_Pin,GPIO_PIN_SET)
#define WS2812B_LOW	   HAL_GPIO_WritePin(WS2812B_GPIO_Port,WS2812B_Pin,GPIO_PIN_RESET)

#define WS2812B_ARR 160		
#define T0H 32					
#define T1H 128					

#define LED_NUM  5	
#define DATA_SIZE  24 	    

void PWM_WS2812B_Init(void);
void WS2812B_Reset(void);
void PWM_WS2812B_Write_24Bits(uint16_t num,uint32_t GRB_Data);
void PWM_WS2812B_Show(uint16_t num);
void PWM_WS2812B_Red(uint16_t num);
void PWM_WS2812B_Green(uint16_t num);
void PWM_WS2812B_Blue(uint16_t num);
void PWM_WS2812B(uint16_t num,u32 RGB_data);
void LEDUpdate(void);

#endif

