#include "apply.h"
#include "tmc5130.h"
#include "hx711.h"
#include "led.h"
#include "heat.h"
#include "bq27441.h"
#include "bq25895.h"
#include "screen.h"

s32 ForceRawOffset;//应变片偏置
s32 ForceRawActual;//应变片实际值

u8 PWR_STATE=0;

u8 WorkMode=0;
/*
0:等待
0000 0001:单加热准备
0000 0010:单脉动准备
0000 0011:自动准备
0000 0100:
0000 0101:单加热运行
0000 0110:单脉动运行
0000 0111:自动运行
4:程序中止
*/
u8 PowerState,LastPowerState;
/*

1:正在充电
2:充电已满
3:未充电电量充足
*/

u8 ForceSet=5;
s32 ForceRawSet=422672;//屏幕设定的值
s32 ForceRawSetAct;//实时设定值

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
				//TMC_ENN(0);//输出使能
				//HAL_TIM_Base_Start_IT(&htim6);	
				break;
			
			/*单加热准备->单加热运行*/
			case 0x01:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				HeatPower(ON);
				break;
			
			/*单脉动准备->单脉动运行*/
			case 0x02:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				TMC_ENN(0);//输出使能
				HAL_TIM_Base_Start_IT(&htim6);
				break;
			
			/*自动准备->自动运行*/
			case 0x03:
				WorkMode |= 0x04;
				ScreenTimerStart(WorkMode);
				HeatPower(ON);
				TMC_ENN(0);//输出使能
				HAL_TIM_Base_Start_IT(&htim6);
				break;
			
			case 0x04:break;
			
			/*按键停止单加热*/
			case 0x05:
				ScreenWorkModeQuit(WorkMode);
				WorkMode=0;
				HeatPower(OFF);
				HeatPWMVal=0;
				break;
				
			/*按键停止单脉动*/
			case 0x06:
				ScreenWorkModeQuit(WorkMode);
				WorkMode=0;
				HAL_TIM_Base_Stop_IT(&htim6);
				Tim6Cnt=0;
				MotorState=0;
				MotorChecking();
				break;
			
			/*按键停止自动*/
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
		/*热敷开始*/
		case 0x1041:
			WorkMode = 0x01;
			break;
		
		/*热敷结束*/
		case 0x1030:
			WorkMode=0;
			HeatPower(OFF);
			HeatPWMVal=0;
			break;
		
		/*脉动开始*/
		case 0x1005:
			ForceSet=ScreenCmdData/50;//设定压力
			ForceRawSet=ForceSet*HX711_SCALE_FACTOR;
			WorkMode = 0x02;
			break;
			
		/*脉动结束*/
		case 0x1034:
			WorkMode=0;
			HAL_TIM_Base_Stop_IT(&htim6);
			Tim6Cnt=0;
			MotorState=0;
			MotorChecking();
			break;
		
		/*自动模式开始*/
		case 0x1037:
			ForceSet=ScreenCmdData/50;//设定压力
			ForceRawSet=ForceSet*HX711_SCALE_FACTOR;
			WorkMode = 0x03;
			break;
		
		/*自动模式结束*/
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
		BQ25895_Write(0x09,0x64);//关断BATFET
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























