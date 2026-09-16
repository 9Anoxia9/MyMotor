#ifndef __CRM_H__
#define __CRM_H__

#if __cplusplus
extern "C"{
#endif

#include "at32f45x.h"

#define MHz                 1000000
#define SYSCLK_FRQ          (192*MHz)
#define APB1CLK_FEQ         SYSCLK_FRQ
#define APB2CLK_FEQ         SYSCLK_FRQ
#define APB3CLK_FEQ         (48*MHz)



#define OUTPUT_SCLK_FRQ     0

void system_clock_init(void);

#if __cplusplus
}
#endif

#endif
