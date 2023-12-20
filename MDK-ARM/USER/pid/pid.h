#ifndef _PID_H
#define _PID_H
#include <sys.h>	  


#define Limit(x,min,max) (x)=(((x)<=(min))?(min):(((x)>=(max))?(max):(x)))

typedef struct
{
    float target_val;
    float actual_val;
    float err;
    float err_last; 
    float Kp,Ki,Kd;
    float integral;
    float maxIntegral;
    float maxOutput;
}PID_typedef;

void HeatPIDInit(void);
u8 PID_realize(PID_typedef *pid,float temp_val) ;


#endif

