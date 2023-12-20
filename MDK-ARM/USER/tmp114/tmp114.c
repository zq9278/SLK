#include "tmp114.h"
#include "delay.h"


u8 EyeTmpRaw[2];
float EyeTmp;

extern I2C_HandleTypeDef hi2c2;



void TMP114_Init(void)
{ 
	
}  

void TMP114_Read(u8 ReadAddr,u8* pBuffer)   
{ 	
	HAL_I2C_Mem_Read_DMA(&hi2c2, TMP114Address, ReadAddr,I2C_MEMADD_SIZE_8BIT, pBuffer,2);
}  

void TMP114_MultiRead(u8* pBuffer)   
{ 	
	HAL_I2C_Mem_Read_DMA(&hi2c2, TMP114Address, 0x00,I2C_MEMADD_SIZE_8BIT, pBuffer,0x14);
}  
 
void TMP114_WriteByte(u8 WriteAddr,u8 WriteData)
{
	u8 Data[1];
	Data[0]=WriteData;
	HAL_I2C_Mem_Write_DMA(&hi2c2, TMP114Address, WriteAddr,I2C_MEMADD_SIZE_8BIT, Data, 1);
} 

void TMP114_WriteWord(u8 WriteAddr,u16 WriteData)
{
	u8 Data[2];
	Data[0]=WriteData;
	Data[1]=WriteData>>8;
	HAL_I2C_Mem_Write_DMA(&hi2c2, TMP114Address, WriteAddr,I2C_MEMADD_SIZE_8BIT, Data, 2);
	delay_ms(2);
} 

float TmpRaw2Ture(u8* TmpRawData)
{
	s16 TmpData;
	TmpData=(TmpRawData[0]<<8) | TmpRawData[1];
	return TmpData*0.0078125;
}
