#ifndef __TMP114_H
#define __TMP114_H
#include "sys.h"




void TMP114_Init(void);
void TMP114_Read(u8 ReadAddr,u8* pBuffer);
void TMP114_MultiRead(u8* pBuffer);
void TMP114_WriteByte(u8 WriteAddr,u8 WriteData);
void TMP114_WriteWord(u8 WriteAddr,u16 WriteData);
float TmpRaw2Ture(u8* TmpRawData);


		 
#endif

