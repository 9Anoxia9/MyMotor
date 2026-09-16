#ifndef __TIM_H__
#define __TIM_H__


#if __cplusplus
extern "C" {
#endif

#include "at32f45x.h"
#include "crm.h"

//TIM1\TIM8
#define TIM1_8_SRC_FRQ      APB2CLK_FEQ
#define TIM1_8_FRQ          24000
#define TIM1_8_PERIOD       (APB2CLK_FEQ/(TIM1_8_FRQ<<1))       //20.8us
#define TIM1_8_RPR          2                                   //62.5us
#define TIM1_8_DEADTIME     100
#define TIM1_CNT_START      (TIM1_8_PERIOD/2)

#define TIM13_FRQ           20000
#define TIM13_PERIOD        (APB1CLK_FEQ/(TIM13_FRQ))
#define TIM13_DEATIME       100

#define TIM14_PERIOD        (TIM1_8_PERIOD*(TIM1_8_RPR+1)*2)    //125us
#define TIM14_CNT_START     TIM1_CNT_START

#define CNT_UP_DIR          0
#define CNT_DW_DIR          1



void board_tim1_init();
void board_tim8_init();
void board_tim13_init();
void board_tim14_init();
uint8_t get_tmr_cnt_dir(tmr_type *tmr_x);

#if __cplusplus
}
#endif

#endif
