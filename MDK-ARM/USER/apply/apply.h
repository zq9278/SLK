#ifndef _APPLY_H
#define _APPLY_H
#include <sys.h>	  

void SW_CMDHandler(void);
void TaskProcessing(void);
void PowerStateUpdate(void);
void UART1_CMDHandler(u8 *DATA);
void BATCheckDIS(void);
void SystemPowerDown(void);
	
#endif

