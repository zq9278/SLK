#include "screen.h"
#include "math.h"
#include "stdlib.h"
#include "hx711.h"


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
//#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART4->ISR&0X40)==0);//ѭ������,ֱ���������   
	USART4->TDR=(u8)ch;      
	return ch;
}
#endif 
u8 USART1_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART1_RX_STA=0;       //����״̬���	
u8 SendBuff[15];

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���

u16 ScreenCmdAdd,ScreenCmdData;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern u8 WorkMode;


//��ʼ��IO ����1 
//bound:������
//void uart_init1(u32 bound)
//{	
//	//UART ��ʼ������
//	__HAL_RCC_USART1_CLK_ENABLE();
//	UART1_Handler.Instance=USART1;					    //USART1
//	UART1_Handler.Init.BaudRate=bound;				    //������
//	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
//	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
//	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
//	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
//	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
//	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()��ʹ��UART1
//	
////	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
//  __HAL_UART_ENABLE_IT(&UART1_Handler,UART_IT_RXNE);
//	__HAL_UART_CLEAR_FLAG(&UART1_Handler,UART_FLAG_TC);
//}



//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��

//void HAL_UART_MspInit(UART_HandleTypeDef *huart)
//{
//    //GPIO�˿�����
//	GPIO_InitTypeDef GPIO_Initure;
//	
//	if(huart->Instance==USART1)//����Ǵ���1�����д���1 MSP��ʼ��
//	{
//		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
//		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
//	
//		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
//		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
//		GPIO_Initure.Pull=GPIO_PULLUP;			//����
//		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
//		GPIO_Initure.Alternate=GPIO_AF1_USART1;	//����ΪUSART1
//		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA9

//		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
//		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA10
//		
//#if EN_USART1_RX
//		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
//		HAL_NVIC_SetPriority(USART1_IRQn,1,1);			//��ռ���ȼ�1�������ȼ�1
//#endif	
//	}
//	

//}

void SendDataToScreen(u8* Data,u8 len)
{

	HAL_UART_Transmit(&huart1,(uint8_t*)Data,len,1000);	
	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
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
