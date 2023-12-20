#include "apply.h"
#include "tmc5130.h"
#include "hx711.h"
#include "led.h"
#include "heat.h"
#include "bq27441.h"
#include "bq25895.h"
#include "screen.h"

s32 ForceRawOffset;//Ӧ��Ƭƫ��
s32 ForceRawActual;//Ӧ��Ƭʵ��ֵ

u8 PWR_STATE=0;

u8 WorkMode=0;
/*
0:�ȴ�
0000 0001:������׼��
0000 0010:������׼��
0000 0011:�Զ�׼��
0000 0100:
0000 0101:����������
0000 0110:����������
0000 0111:�Զ�����
4:������ֹ
*/
u8 PowerState,LastPowerState;
/*

1:���ڳ��
2:�������
3:δ����������
*/

u8 ForceSet=5;
s32 ForceRawSet=422672;//��Ļ�趨��ֵ
s32 ForceRawSetAct;//ʵʱ�趨ֵ

u8 MotorState;
u8 MotorCompareState;

extern uint8_t SW_CNT_Flag;
extern TIM_HandleTypeDef htim6;
extern u16 Tim6Cnt;

extern u8 BQ25895Reg[21];

extern u16 ScreenCmdAdd,ScreenCmdData;

extern u8 HeatPWMVal;

extern BQ27441_typedef BQ27441;

void SW_CMDHandler(void)
{
	if(SW_CNT_Flag)
	{
		SW_CNT_Flag=0;
		switch(WorkMode)
		{
			/**/
			case 0:
				//WorkMode |=0x06;
				//TMC_ENN(0);//���ʹ��
				//HAL_TIM_Base_Start_IT(&htim6);	
				break;
			
			/*������׼��->����������*/
			case 0x01:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				HeatPower(ON);
				break;
			
			/*������׼��->����������*/
			case 0x02:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				TMC_ENN(0);//���ʹ��
				HAL_TIM_Base_Start_IT(&htim6);
				break;
			
			/*�Զ�׼��->�Զ�����*/
			case 0x03:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				HeatPower(ON);
				TMC_ENN(0);//���ʹ��
				HAL_TIM_Base_Start_IT(&htim6);
				break;
			
			case 0x04:break;
			
			/*����ֹͣ������*/
			case 0x05:
				ScreenWorkModeQuit(WorkMode);
				WorkMode=0;
				HeatPower(OFF);
				HeatPWMVal=0;
				break;
				
			/*����ֹͣ������*/
			case 0x06:
				ScreenWorkModeQuit(WorkMode);
				WorkMode=0;
				HAL_TIM_Base_Stop_IT(&htim6);
				Tim6Cnt=0;
				MotorState=0;
				MotorChecking();
				break;
			
			/*����ֹͣ�Զ�*/
			case 0x07:
				ScreenWorkModeQuit(WorkMode);
				WorkMode=0;
				HeatPower(OFF);
				HeatPWMVal=0;
				HAL_TIM_Base_Stop_IT(&htim6);
				Tim6Cnt=0;
				MotorState=0;
				MotorChecking();
				break;
			
			default:break;
		}
	}
}

void TaskProcessing(void)
{
	switch(WorkMode)
	{
		case 0:break;
		case 1:
			break;
		case 6:
			ForceRawActual=HX711_Read();
//			MotorCompare(ForceRawOffset+422672,ForceRawActual);//5N
			MotorCompareState=MotorCompare(ForceRawOffset+ForceRawSetAct,ForceRawActual);
			break;
		case 7:
			ForceRawActual=HX711_Read();
//			MotorCompare(ForceRawOffset+422672,ForceRawActual);//5N
			MotorCompareState=MotorCompare(ForceRawOffset+ForceRawSetAct,ForceRawActual);
			break;
		default:break;
	}
}

void PowerStateUpdate(void)
{
	u8 CHRG_STAT;
	CHRG_STAT=(BQ25895Reg[0x0b]&0x18)>>3;
	if(CHRG_STAT==1||CHRG_STAT==2)//Pre-charge Fast Charging
	{
		PowerState=1;
	}
	else if(CHRG_STAT==3)//Charge Termination Done
	{
		PowerState=2;
	}
	else if(CHRG_STAT==0)//Not Charging
	{
		PowerState=3;
	}
}

void UART1_CMDHandler(u8 *DATA)
{
	ScreenCmdAdd=(DATA[2]<<8)|DATA[3];
	ScreenCmdData=(DATA[5]<<8)|DATA[6];
	switch(ScreenCmdAdd)
	{
		/*�ȷ�ʼ*/
		case 0x1041:
			WorkMode = 0x01;
			break;
		
		/*�ȷ����*/
		case 0x1030:
			WorkMode=0;
			HeatPower(OFF);
			HeatPWMVal=0;
			break;
		
		/*������ʼ*/
		case 0x1005:
			ForceSet=ScreenCmdData/50;//�趨ѹ��
			ForceRawSet=ForceSet*HX711_SCALE_FACTOR;
			WorkMode = 0x02;
			break;
			
		/*��������*/
		case 0x1034:
			WorkMode=0;
			HAL_TIM_Base_Stop_IT(&htim6);
			Tim6Cnt=0;
			MotorState=0;
			MotorChecking();
			break;
		
		/*�Զ�ģʽ��ʼ*/
		case 0x1037:
			ForceSet=ScreenCmdData/50;//�趨ѹ��
			ForceRawSet=ForceSet*HX711_SCALE_FACTOR;
			WorkMode = 0x03;
			break;
		
		/*�Զ�ģʽ����*/
		case 0x1038:
			WorkMode=0;
			HeatPower(OFF);
			HeatPWMVal=0;
			HAL_TIM_Base_Stop_IT(&htim6);
			Tim6Cnt=0;
			MotorState=0;
			MotorChecking();
			break;

		
			
		default:break;	
		}				
}

void BATCheckDIS(void)
{
	if(BQ27441.Voltage<=3000)
	{
		BQ25895_Write(0x09,0x64);//�ض�BATFET
	}
}
void SystemPowerDown(void)
{
	WorkMode=0;
	HeatPower(OFF);
	HeatPWMVal=0;
	HAL_TIM_Base_Stop_IT(&htim6);
	Tim6Cnt=0;
	MotorState=0;
}























