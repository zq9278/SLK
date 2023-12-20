#ifndef __BQ27441_H
#define __BQ27441_H
#include "sys.h"




typedef struct
{
  u16 Voltage;
	u16 Temperature;
	s16 AvgCurrent;
	u16 SOC;
	u16 FullChargeCapacity;
}BQ27441_typedef;


u8 BQ27441_Init(void);
void BQ27441_Read(u8 ReadAddr,u8* pBuffer);
void BQ27441_MultiRead(BQ27441_typedef *BQ_State);
void BQ27441_WriteByte(u8 WriteAddr,u8 WriteData);
void BQ27441_WriteWord(u8 WriteAddr,u16 WriteData);


		 
#endif

