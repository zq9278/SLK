#ifndef _SCREEN_H
#define _SCREEN_H
#include "sys.h"
#include "stdio.h"	

#define USART_REC_LEN  			20  	//定义最大接收字节数 200

#define False	0
#define Ture	1
	  	
extern u8  USART1_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART1_RX_STA;       //接收状态标记	

#define RXBUFFERSIZE   1 //缓存大小
extern u8 aRxBuffer[RXBUFFERSIZE];//HAL库USART接收Buffer

void SendDataToScreen(u8* Data,u8 len);
void ScreenUpdateTemperature(float value);
void ScreenUpdateForce(u32 value);
void ScreenUpdateSOC(u16 value,u8 state);
void ScreenWorkModeQuit(u8 workmodenumber);
void ScreenTimerStart(u8 workmodenumber);
#endif
