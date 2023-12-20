#ifndef __BQ25895_H
#define __BQ25895_H
#include "sys.h"




void BQ25895_Init(void);
void BQ25895_Read(u8 ReadAddr,u8* pBuffer);
void BQ25895_MultiRead(u8* pBuffer);
void BQ25895_Write(u8 WriteAddr,u8 WriteData);



		 
#endif

