#include "tim.h"
#include "gpio.h"

//控制M0
void board_tim1_init(){

  tmr_output_config_type  tmr_output_struct;
  tmr_brkdt_config_type tmr_brkdt_config_struct;
  
  crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);

  m0_pwm_gpio_init();

  tmr_base_init(TMR1, TIM1_8_PERIOD, 0);              //频率24K
  tmr_cnt_dir_set(TMR1, TMR_COUNT_TWO_WAY_3);         //中央对齐模式3

  tmr_repetition_counter_set(TMR1,TIM1_8_RPR);        //重复计数次数为3

  /* channel 1, 2, 3 and 4 configuration in output mode */
  tmr_output_default_para_init(&tmr_output_struct);       
  tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_B;
  tmr_output_struct.oc_output_state = TRUE;
  tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;   //参考电平等于极性电平,就输出高,否则输出低
  tmr_output_struct.oc_idle_state = FALSE;
  tmr_output_struct.occ_output_state = TRUE;
  tmr_output_struct.occ_polarity = TMR_OUTPUT_ACTIVE_LOW;   //C1C的参考电机会取反1次，取反后的电平与极性电平一致，则输出为高，否则为低
  tmr_output_struct.occ_idle_state = TRUE;
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, (TIM1_8_PERIOD>>1) );
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_2, (TIM1_8_PERIOD>>1) );
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_3, (TIM1_8_PERIOD>>1) );

  /* automatic output enable, stop, dead time and lock configuration*/
  tmr_brkdt_default_para_init(&tmr_brkdt_config_struct);
  tmr_brkdt_config_struct.brk_enable = FALSE;
  tmr_brkdt_config_struct.auto_output_enable = FALSE;
  tmr_brkdt_config_struct.deadtime = TIM1_8_DEADTIME;
  tmr_brkdt_config_struct.fcsodis_state = TRUE;
  tmr_brkdt_config_struct.fcsoen_state = FALSE;
  tmr_brkdt_config_struct.wp_level = TMR_WP_LEVEL_3;
  tmr_brkdt_config(TMR1, &tmr_brkdt_config_struct);

  /*开启通道CxDT更新缓存,写入新值到CxDT并不会直接更新CxDT,而是存入影子寄存器
    在定时器发生溢出事件时更新CxDT.
  */
  tmr_output_channel_buffer_enable(TMR1,TMR_SELECT_CHANNEL_1,TRUE);
  tmr_output_channel_buffer_enable(TMR1,TMR_SELECT_CHANNEL_2,TRUE);
  tmr_output_channel_buffer_enable(TMR1,TMR_SELECT_CHANNEL_3,TRUE);

  tmr_primary_mode_select(TMR1, TMR_PRIMARY_SEL_OVERFLOW);

  /* tmr1 output enable */
  tmr_output_enable(TMR1, TRUE);
  /* enable tmr1 */
  tmr_counter_enable(TMR1, FALSE);
}

//控制M1
void board_tim8_init(){

  tmr_output_config_type  tmr_output_struct;
  tmr_brkdt_config_type   tmr_brkdt_config_struct;
  
  crm_periph_clock_enable(CRM_TMR8_PERIPH_CLOCK, TRUE);

  m1_pwm_gpio_init();

  tmr_base_init(TMR8, TIM1_8_PERIOD, 0);              //频率24K
  tmr_cnt_dir_set(TMR8, TMR_COUNT_TWO_WAY_3);         //中央对齐模式3

  tmr_repetition_counter_set(TMR8,TIM1_8_RPR);        //重复计数次数为3

  /* channel 1, 2, 3 and 4 configuration in output mode */
  tmr_output_default_para_init(&tmr_output_struct);       
  tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_B;
  tmr_output_struct.oc_output_state = TRUE;
  tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
  tmr_output_struct.oc_idle_state = FALSE;
  tmr_output_struct.occ_output_state = TRUE;
  tmr_output_struct.occ_polarity = TMR_OUTPUT_ACTIVE_LOW;
  tmr_output_struct.occ_idle_state = TRUE;
  tmr_output_channel_config(TMR8, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
  tmr_output_channel_config(TMR8, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
  tmr_output_channel_config(TMR8, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
  tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, (TIM1_8_PERIOD>>1) );
  tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, (TIM1_8_PERIOD>>1) );
  tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, (TIM1_8_PERIOD>>1) );
  
  /* automatic output enable, stop, dead time and lock configuration*/
  tmr_brkdt_default_para_init(&tmr_brkdt_config_struct);
  tmr_brkdt_config_struct.brk_enable = FALSE;
  tmr_brkdt_config_struct.auto_output_enable = FALSE;
  tmr_brkdt_config_struct.deadtime = TIM1_8_DEADTIME;
  tmr_brkdt_config_struct.fcsodis_state = TRUE;
  tmr_brkdt_config_struct.fcsoen_state = FALSE;
  tmr_brkdt_config_struct.wp_level = TMR_WP_LEVEL_3;
  tmr_brkdt_config(TMR8, &tmr_brkdt_config_struct);
  
  /*开启通道CxDT更新缓存,写入新值到CxDT并不会直接更新CxDT,而是存入影子寄存器
    在定时器发生溢出事件时更新CxDT.
  */
  tmr_output_channel_buffer_enable(TMR8,TMR_SELECT_CHANNEL_1,TRUE);
  tmr_output_channel_buffer_enable(TMR8,TMR_SELECT_CHANNEL_2,TRUE);
  tmr_output_channel_buffer_enable(TMR8,TMR_SELECT_CHANNEL_3,TRUE);

  //定时器复位
  tmr_primary_mode_select(TMR8, TMR_PRIMARY_SEL_OVERFLOW);

  /* tmr8 output enable */
  tmr_output_enable(TMR8, TRUE);
  /* enable tmr8 */
  tmr_counter_enable(TMR8, FALSE);
}

//控制泄放电阻
void board_tim13_init(){

  tmr_output_config_type  tmr_output_struct;
  tmr_brkdt_config_type   tmr_brkdt_config_struct;

  crm_periph_clock_enable(CRM_TMR13_PERIPH_CLOCK, TRUE);

  aux_pwm_gpio_init();

  /* tmr13 time base configuration */
  tmr_base_init(TMR13, TIM13_PERIOD, 0);
  tmr_cnt_dir_set(TMR13, TMR_COUNT_UP);

  tmr_output_default_para_init(&tmr_output_struct);
  tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_B;
  tmr_output_struct.oc_output_state = TRUE;
  tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
  tmr_output_struct.oc_idle_state = FALSE;
  tmr_output_struct.occ_output_state = TRUE;
  tmr_output_struct.occ_polarity = TMR_OUTPUT_ACTIVE_LOW;
  tmr_output_struct.occ_idle_state = TRUE;
  tmr_output_channel_config(TMR13, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
  tmr_channel_value_set(TMR13, TMR_SELECT_CHANNEL_1, (TIM13_PERIOD>>1));

  /* automatic output enable, stop, dead time and lock configuration*/
  tmr_brkdt_default_para_init(&tmr_brkdt_config_struct);
  tmr_brkdt_config_struct.brk_enable = FALSE;
  tmr_brkdt_config_struct.auto_output_enable = FALSE;
  tmr_brkdt_config_struct.deadtime = TIM13_DEATIME;
  tmr_brkdt_config_struct.fcsodis_state = TRUE;
  tmr_brkdt_config_struct.fcsoen_state = FALSE;
  tmr_brkdt_config_struct.wp_level = TMR_WP_LEVEL_3;
  tmr_brkdt_config(TMR13, &tmr_brkdt_config_struct);

  tmr_output_channel_buffer_enable(TMR13, TMR_SELECT_CHANNEL_1, FALSE);

  /* tmr enable output */
  tmr_output_enable(TMR13, FALSE);

  /* tmr enable counter */
  tmr_counter_enable(TMR13, TRUE);
}

//用于软件耗时计算
void board_tim14_init(){

    crm_periph_clock_enable(CRM_TMR14_PERIPH_CLOCK, TRUE);

#if 0
    tmr_interrupt_enable(TMR14,TMR_OVF_INT,TRUE);
    nvic_irq_enable(TMR8_TRG_HALL_TMR14_IRQn, 1, 0);
#endif    

    /* tmr13 time base configuration */
    tmr_base_init(TMR14, TIM14_PERIOD, 0);
    tmr_cnt_dir_set(TMR14, TMR_COUNT_UP);

    /* tmr enable counter */
    tmr_counter_enable(TMR14, FALSE);
}

uint8_t get_tmr_cnt_dir(tmr_type *tmr_x){
  uint8_t cnt_dir = tmr_x->ctrl1_bit.cnt_dir & 0x01;
  return cnt_dir;
}

