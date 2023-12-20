#include "bq27441.h"
#include "delay.h"
#include "24cxx.h"

u8 BQ27441_TempData[2];

u8 NEW_DC_MSB=0x0A,NEW_DC_LSB=0x8C;//2700mAh
u8 NEW_DE_MSB=0x27,NEW_DE_LSB=0x06;//9900mWh
u8 NEW_TV_MSB=0x0B,NEW_TV_LSB=0xB8;//3000mV
u8 NEW_TR_MSB=0x01,NEW_TR_LSB=0xA5;//2700mAh/(0.1*64mA)。64mA为BQ25895充电关断电流
u8 OLD_DC_LSB,OLD_DC_MSB,OLD_DE_LSB,OLD_DE_MSB,OLD_TV_LSB,OLD_TV_MSB,OLD_TR_LSB,OLD_TR_MSB;
u8 CHKSUM;
u8 BQ27441_FLAG_LSB,BQ27441_CONTROL_STATUS_MSB;

BQ27441_typedef BQ27441;

extern I2C_HandleTypeDef hi2c1;



u8 BQ27441_Init(void)
{ 
	u8 OLD_CHKSUM,NEW_CHKSUM,TMP_CHKSUM;
	u8 tempee;
	tempee=AT24CXX_ReadOneByte(EEPROM_BQ27441Config_Add);
	if(tempee!=0x55)
	{
	BQ27441_WriteWord(0x00,0x8000);
	BQ27441_WriteWord(0x00,0x8000);
	BQ27441_WriteWord(0x00,0x0013);
	delay_ms(2000);
	
	BQ27441_Read(0x06,&BQ27441_FLAG_LSB);
	delay_ms(1);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	if((BQ27441_FLAG_LSB&0x10) != 0x10)
	{
		return 0;
	}
	BQ27441_WriteByte(0x61,0x00);
	BQ27441_WriteByte(0x3E,0x52);
	BQ27441_WriteByte(0x3F,0x00);
	delay_ms(1);
	BQ27441_Read(0x60,&OLD_CHKSUM);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	TMP_CHKSUM=0xff-OLD_CHKSUM;
	BQ27441_Read(0x4A,&OLD_DC_MSB);
	BQ27441_Read(0x4B,&OLD_DC_LSB);
	BQ27441_Read(0x4C,&OLD_DE_MSB);
	BQ27441_Read(0x4D,&OLD_DE_LSB);
	BQ27441_Read(0x50,&OLD_TV_MSB);
	BQ27441_Read(0x51,&OLD_TV_LSB);
	BQ27441_Read(0x5B,&OLD_TR_MSB);
	BQ27441_Read(0x5C,&OLD_TR_LSB);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	if(OLD_DC_MSB != NEW_DC_MSB || OLD_DC_LSB != NEW_DC_LSB)//读取电池容量数据，与设定一样就不用重新设定
	{	
		TMP_CHKSUM=TMP_CHKSUM-OLD_DC_LSB-OLD_DC_MSB-OLD_DE_LSB-
			OLD_DE_MSB-OLD_TV_LSB-OLD_TV_MSB-OLD_TR_LSB-OLD_TR_MSB;
		delay_ms(1);
		BQ27441_WriteByte(0x4A,NEW_DC_MSB);
		BQ27441_WriteByte(0x4B,NEW_DC_LSB);
		BQ27441_WriteByte(0x4C,NEW_DE_MSB);
		BQ27441_WriteByte(0x4D,NEW_DE_LSB);
		BQ27441_WriteByte(0x50,NEW_TV_MSB);
		BQ27441_WriteByte(0x51,NEW_TV_LSB);
		BQ27441_WriteByte(0x5B,NEW_TR_MSB);
		BQ27441_WriteByte(0x5C,NEW_TR_LSB);
		TMP_CHKSUM=TMP_CHKSUM+NEW_DC_LSB+NEW_DC_MSB+NEW_DE_LSB+
			NEW_DE_MSB+NEW_TV_LSB+NEW_TV_MSB+NEW_TR_LSB+NEW_TR_MSB;
		NEW_CHKSUM=0xff-TMP_CHKSUM;
		BQ27441_WriteByte(0x60,NEW_CHKSUM);
		delay_ms(1000);
		BQ27441_WriteByte(0x3E,0x52);
		BQ27441_WriteByte(0x3F,0x00);
		delay_ms(1);
		BQ27441_Read(0x60,&CHKSUM);
		while(hi2c1.State != HAL_I2C_STATE_READY){;}
		if(CHKSUM==NEW_CHKSUM)
		{
			do
			{
				BQ27441_WriteWord(0x00,0x0042);
				delay_ms(1);
				BQ27441_Read(0x06,&BQ27441_FLAG_LSB);
				delay_ms(1);
			}while((BQ27441_FLAG_LSB&0x10) == 0x10);
			BQ27441_WriteWord(0x00,0x0020);
			AT24CXX_WriteOneByte(EEPROM_BQ27441Config_Add,0x55);//配置完成后写入EEPROM
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		do
		{
			BQ27441_WriteWord(0x00,0x0042);
			delay_ms(1);
			BQ27441_Read(0x06,&BQ27441_FLAG_LSB);
			delay_ms(1);
		}while((BQ27441_FLAG_LSB&0x10) == 0x10);
		BQ27441_WriteWord(0x00,0x0020);
		AT24CXX_WriteOneByte(EEPROM_BQ27441Config_Add,0x55);//配置完成后写入EEPROM
		return 1;
	}
	}
	else
	{
 		return 2;//已经配置过
	}
}  

void BQ27441_Read(u8 ReadAddr,u8* pBuffer)   
{ 	
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, ReadAddr,I2C_MEMADD_SIZE_8BIT, pBuffer,1);
}  

void BQ27441_MultiRead(BQ27441_typedef *BQ_State)   
{ 	
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, 0x04,I2C_MEMADD_SIZE_8BIT, (u8*)&(BQ_State->Voltage),2);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, 0x02,I2C_MEMADD_SIZE_8BIT, (u8*)&(BQ_State->Temperature),2);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, 0x10,I2C_MEMADD_SIZE_8BIT, (u8*)&(BQ_State->AvgCurrent),2);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, 0x1C,I2C_MEMADD_SIZE_8BIT, (u8*)&(BQ_State->SOC),2);
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	HAL_I2C_Mem_Read_DMA(&hi2c1, BQ27441Address, 0x0E,I2C_MEMADD_SIZE_8BIT, (u8*)&(BQ_State->FullChargeCapacity),2);
}  
 
void BQ27441_WriteByte(u8 WriteAddr,u8 WriteData)
{
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	BQ27441_TempData[0]=WriteData;
	HAL_I2C_Mem_Write_DMA(&hi2c1, BQ27441Address, WriteAddr,I2C_MEMADD_SIZE_8BIT, BQ27441_TempData, 1);
} 

void BQ27441_WriteWord(u8 WriteAddr,u16 WriteData)
{
	while(hi2c1.State != HAL_I2C_STATE_READY){;}
	BQ27441_TempData[0]=WriteData;
	BQ27441_TempData[1]=WriteData>>8;
	HAL_I2C_Mem_Write_DMA(&hi2c1, BQ27441Address, WriteAddr,I2C_MEMADD_SIZE_8BIT, BQ27441_TempData, 2);
} 

