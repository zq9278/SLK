#include "screen.h"
#include "math.h"
#include "stdlib.h"
#include "hx711.h"


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
//#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART4->ISR&0X40)==0);//循环发送,直到发送完毕   
	USART4->TDR=(u8)ch;      
	return ch;
}
#endif 
u8 USART1_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART1_RX_STA=0;       //接收状态标记	
u8 SendBuff[15];

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲

u16 ScreenCmdAdd,ScreenCmdData;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern u8 WorkMode;


//初始化IO 串口1 
//bound:波特率
//void uart_init1(u32 bound)
//{	
//	//UART 初始化设置
//	__HAL_RCC_USART1_CLK_ENABLE();
//	UART1_Handler.Instance=USART1;					    //USART1
//	UART1_Handler.Init.BaudRate=bound;				    //波特率
//	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
//	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
//	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
//	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
//	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
//	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()会使能UART1
//	
////	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
//  __HAL_UART_ENABLE_IT(&UART1_Handler,UART_IT_RXNE);
//	__HAL_UART_CLEAR_FLAG(&UART1_Handler,UART_FLAG_TC);
//}



//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄

//void HAL_UART_MspInit(UART_HandleTypeDef *huart)
//{
//    //GPIO端口设置
//	GPIO_InitTypeDef GPIO_Initure;
//	
//	if(huart->Instance==USART1)//如果是串口1，进行串口1 MSP初始化
//	{
//		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
//		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
//	
//		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
//		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
//		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
//		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
//		GPIO_Initure.Alternate=GPIO_AF1_USART1;	//复用为USART1
//		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA9

//		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
//		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA10
//		
//#if EN_USART1_RX
//		HAL_NVIC_EnableIRQ(USART1_IRQn);				//使能USART1中断通道
//		HAL_NVIC_SetPriority(USART1_IRQn,1,1);			//抢占优先级1，子优先级1
//#endif	
//	}
//	

//}

void SendDataToScreen(u8* Data,u8 len)
{

	HAL_UART_Transmit(&huart1,(uint8_t*)Data,len,1000);	
	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)!=SET);		//等待发送结束
}

u16 Forcevalue;

void ScreenUpdateForce(u32 value)
{
	
	
	Forcevalue=(u16)(value/HX711_SCALE_FACTOR_10*6);
	
	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
	if(WorkMode==0x06)
	{
		SendBuff[4]=0x07;
		SendBuff[6]=0x02;
	}
	else if(WorkMode==0x07)
	{
		SendBuff[4]=0x0C;
		SendBuff[6]=0x04;
	}
	SendBuff[0]=0xEE;
	SendBuff[1]=0xB1;
	SendBuff[2]=0x10;
	SendBuff[3]=0x00;
	
	SendBuff[5]=0x00;
	
	SendBuff[7]=Forcevalue/100+'0';
	SendBuff[8]=Forcevalue/10%10+'0';
	SendBuff[9]=Forcevalue%10+'0';
	SendBuff[10]=0xFF;
	SendBuff[11]=0xFC;
	SendBuff[12]=0xFF;
	SendBuff[13]=0xFF;
	
	
	HAL_UART_Transmit_DMA(&huart1,SendBuff,14);	
}





void ScreenUpdateTemperature(float value)
{
	u16 Tmpvalue;
	
	Tmpvalue=value+0.5f;
	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
	if(WorkMode==0x05)
	{
		SendBuff[4]=0x03;
		SendBuff[6]=0x02;
	}
	else if(WorkMode==0x07)
	{
		SendBuff[4]=0x0C;
		SendBuff[6]=0x03;
	}
	SendBuff[0]=0xEE;
	SendBuff[1]=0xB1;
	SendBuff[2]=0x10;
	SendBuff[3]=0x00;
	
	SendBuff[5]=0x00;
	
	SendBuff[7]=Tmpvalue/10+'0';
	SendBuff[8]=Tmpvalue%10+'0';
	SendBuff[9]=0xFF;
	SendBuff[10]=0xFC;
	SendBuff[11]=0xFF;
	SendBuff[12]=0xFF;
	
	HAL_UART_Transmit_DMA(&huart1,SendBuff,13);	
	
//	u16 Tmpvalue;
//	
//	Tmpvalue=value+0.5f;
//	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
//	if(WorkMode==0x05)
//	{
//		SendBuff[4]=0x03;
//	}
//	else if(WorkMode==0x07)
//	{
//		SendBuff[4]=0x0C;
//	}
//	SendBuff[0]=0xEE;
//	SendBuff[1]=0xB1;
//	SendBuff[2]=0x32;
//	SendBuff[3]=0x00;
//	
//	SendBuff[5]=0x00;
//	SendBuff[6]=0x02;
//	SendBuff[7]=0x00;
//	SendBuff[8]=0x00;
//	SendBuff[9]=0x01;
//	SendBuff[10]=Tmpvalue;
//	SendBuff[11]=0xFF;
//	SendBuff[12]=0xFC;
//	SendBuff[13]=0xFF;
//	SendBuff[14]=0xFF;
//	
//	HAL_UART_Transmit_DMA(&huart1,SendBuff,15);	
}


void ScreenUpdateSOC(u16 value,u8 state)
{
	u8 SOCvalue;
	SOCvalue=value/20;
	if(SOCvalue==5)SOCvalue=4;
	if(state==1 || state==2)SOCvalue+=5;
	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
	SendBuff[0]=0xEE;
	SendBuff[1]=0xB1;
	SendBuff[2]=0x23;
	SendBuff[3]=0x00;
	SendBuff[4]=0x1E;
	SendBuff[5]=0x27;
	SendBuff[6]=0x15;
	SendBuff[7]=SOCvalue;
	SendBuff[8]=0xFF;
	SendBuff[9]=0xFC;
	SendBuff[10]=0xFF;
	SendBuff[11]=0xFF;
	
	HAL_UART_Transmit_DMA(&huart1,SendBuff,12);	
}

void ScreenWorkModeQuit(u8 workmodenumber)
{
	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
	SendBuff[0]=0xEE;
	SendBuff[1]=0xB1;
	SendBuff[2]=0x10;
	SendBuff[3]=0x00;
	if(workmodenumber==0x05)SendBuff[4]=0x03;
	if(workmodenumber==0x06)SendBuff[4]=0x07;
	if(workmodenumber==0x07)SendBuff[4]=0x0C;
	SendBuff[5]=0x00;
	SendBuff[6]=0x07;
	SendBuff[7]=0x32;
	SendBuff[8]=0xFF;
	SendBuff[9]=0xFC;
	SendBuff[10]=0xFF;
	SendBuff[11]=0xFF;
	
	HAL_UART_Transmit_DMA(&huart1,SendBuff,12);	
}

void ScreenTimerStart(u8 workmodenumber)
{
	while(hdma_usart1_tx.State!=HAL_DMA_STATE_READY);
	SendBuff[0]=0xEE;
	SendBuff[1]=0xB1;
	SendBuff[2]=0x10;
	SendBuff[3]=0x00;
	if(workmodenumber==0x05)SendBuff[4]=0x03;
	if(workmodenumber==0x06)SendBuff[4]=0x07;
	if(workmodenumber==0x07)SendBuff[4]=0x0C;
	SendBuff[5]=0x00;
	SendBuff[6]=0x07;
	SendBuff[7]=0x31;
	SendBuff[8]=0xFF;
	SendBuff[9]=0xFC;
	SendBuff[10]=0xFF;
	SendBuff[11]=0xFF;
	
	HAL_UART_Transmit_DMA(&huart1,SendBuff,12);	
}
