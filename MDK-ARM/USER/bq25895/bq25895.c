#include "bq25895.h"
#include "delay.h"


u8 BQ25895Reg[21];
u8 BQ25895TempData[1];


extern I2C_HandleTypeDef hi2c1;



void BQ25895_Init(void)
{ 
	CHG_CE(0);
	BQ25895_Write(0x02,0x7d);//ADC连续转换
	BQ25895_Write(0x03,0x1e);//OTG关闭，最小系统电压设置为3.5V
	BQ25895_Write(0x05,0x10);//充电关断电流设置为64mA
	BQ25895_Write(0x07,0x8d);//关闭看门狗
}  

u8 temp1;
void BQ25895_Read(u8 ReadAddr,u8* pBuffer)   
{ 	
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ25895Address, ReadAddr,I2C_MEMADD_SIZE_8BIT, pBuffer,1);
}  

void BQ25895_MultiRead(u8* pBuffer)   
{ 
	while(hi2c1.State != HAL_I2C_STATE_READY){;}	
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ25895Address, 0x00,I2C_MEMADD_SIZE_8BIT, pBuffer,0x14);
}  
 
void BQ25895_Write(u8 WriteAddr,u8 WriteData)
{
	while(hi2c1.State != HAL_I2C_STATE_READY){;}	
	BQ25895TempData[0]=WriteData;
	HAL_I2C_Mem_Write_DMA(&hi2c1, BQ25895Address, WriteAddr,I2C_MEMADD_SIZE_8BIT, BQ25895TempData, 1);
} 



