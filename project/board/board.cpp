#include "board.h"
#include "main.hpp"
#include <optional>


#define Ctrl_Loop_Handler   TMR5_GLOBAL_IRQHandler
#define Ctrl_Loop_IRQ       TMR5_GLOBAL_IRQn


//多功能IO引脚,要求能灵活切换成各种配置
Gpio gpios[] = {
    {GPIOC,GPIO_PINS_13},   //LED2
    {GPIOC,GPIO_PINS_14},   //GPIO1
    {GPIOC,GPIO_PINS_15},   //GPIO2
    {GPIOH,GPIO_PINS_0},    //GPIO3
    {GPIOH,GPIO_PINS_1},    //GPIO4
    {GPIOA,GPIO_PINS_1},    //GPIO5
    {GPIOC,GPIO_PINS_9},    //CLK_IO
    {GPIOD,GPIO_PINS_2},    //M1_ENC_Z
    {GPIOB,GPIO_PINS_3},    //M0_ENC_Z
};

Gpio clk_io = get_gpio(CLK_IO);

Motor mt(TMR1);

Encoder encoder(spi3_intf);

Control control;

std::array<Axis,AXIS_COUNT> axes{{
    {
        0,
        encoder,
        mt,
        control
    }
}};

Osc osc;

uint32_t _timestamp = 0;

void system_init(void){
    system_clock_init();
}

void board_gpio_init(void){

    enum gpio_mode mode[] = GPIO_DEFAULT_CONFIG;

    for(uint32_t idx = 0;idx < ALL_CONFIGABLE_GPIO;idx++){
        if(!gpios[idx]){
            continue;
        }
        if(mode[idx] == GPIO_INPUT_PULL_NONE){
            gpios[idx].set_input(GPIO_PULL_NONE);
        }else if(mode[idx] == GPIO_INPUT_PULL_UP){
            gpios[idx].set_input(GPIO_PULL_UP);
        }else if(mode[idx] == GPIO_INPUT_PULL_DOWN){
            gpios[idx].set_input(GPIO_PULL_DOWN);
        }else if(mode[idx] == GPIO_OUTPUT_PP){
            gpios[idx].set_output(GPIO_OUTPUT_PUSH_PULL,GPIO_PULL_NONE);
        }else if(mode[idx] == GPIO_OUTPUT_OD_PULL_NONE){
            gpios[idx].set_output(GPIO_OUTPUT_OPEN_DRAIN,GPIO_PULL_NONE);
        }else if(mode[idx] == GPIO_OUTPUT_OD_PULL_UP){
            gpios[idx].set_output(GPIO_OUTPUT_OPEN_DRAIN,GPIO_PULL_UP);  
        }else if(mode[idx] == GPIO_ANALOG){
            gpios[idx].set_analog();
        }
    }
}

void board_init(void){
    
    board_gpio_init();
    board_adc_init();       //ADC1和ADC2初始化
    board_tim1_init();
    board_tim8_init();
    board_tim13_init();
    board_tim14_init();
    board_usb_init();
    board_spi3_init();

    tmr_flag_clear(TMR8,TMR_OVF_FLAG);
    tmr_interrupt_enable(TMR8,TMR_OVF_INT,TRUE);
    nvic_irq_enable(TMR8_OVF_TMR13_IRQn, 1, 0);

    nvic_irq_enable(SPI3_I2S3EXT_IRQn, 0, 0);


    nvic_irq_enable(Ctrl_Loop_IRQ,5,0);
    nvic_irq_enable(OTG_IRQ,6, 0);              //优先级必须比控制环中断低
    //nvic_irq_enable(OTG_WKUP_IRQ, 6, 0);      //优先级必须比控制环中断低
}

void start_timer(){

    /*与TIM8错开1/4个PWM周期 */
    tmr_counter_value_set(TMR1,TIM1_CNT_START);
    /*TIM14与TIM1同步 */
    tmr_counter_value_set(TMR14,TIM14_CNT_START);

    tmr_counter_enable(TMR1, TRUE);
    tmr_counter_enable(TMR8, TRUE);
    tmr_counter_enable(TMR14, TRUE);
}

float phase_i_from_adcval(uint16_t adcval){
    
    int16_t adcval_i = adcval - (1<<11);

    float v_to_resolution = 3.3f/(1<<12);

    return (float)adcval_i*v_to_resolution/AMP_GAIN/MOTOR_CURRENT_RES;
    
}

bool fetch_phase_current(std::optional<I_phUVW>* current0,
                            std::optional<I_phUVW>* current1){

    bool adc_finish = (adc_flag_get(ADC1, ADC_PCCE_FLAG) == SET)
                        && (adc_flag_get(ADC1, ADC_OCCE_FLAG) == SET)
                        && (adc_flag_get(ADC2, ADC_PCCE_FLAG) == SET)
                        && (adc_flag_get(ADC2, ADC_OCCE_FLAG) == SET);

    if(adc_finish == false){
        return adc_finish;
    }

    float tmp_m0_v = adc_preempt_conversion_data_get(ADC1, ADC_PREEMPT_CHANNEL_1);
    float tmp_m0_w = adc_preempt_conversion_data_get(ADC2, ADC_PREEMPT_CHANNEL_1);

    if(get_tmr_cnt_dir(TMR8) == CNT_UP_DIR){
        mt._adc_val.iv = (float)tmp_m0_v;
        mt._adc_val.iw = (float)tmp_m0_w;
    }

    float ib0 = phase_i_from_adcval(tmp_m0_v);
    float ic0 = phase_i_from_adcval(tmp_m0_w);
    float ib1 = phase_i_from_adcval(adc_ordinary_conversion_data_get(ADC1));
    float ic1 = phase_i_from_adcval(adc_ordinary_conversion_data_get(ADC2));

    *current0 = {ib0+ic0,-ib0,-ic0};
    *current1 = {ib1+ic1,-ib1,-ic1};

    return true;
}


size_t board_get_unique_id(uint8_t id[], size_t max_len){
    
    uint32_t *uid32 = (uint32_t *) id;
    uid32[0] = *((uint32_t *)UID_BASE_ADDR);
    uid32[1] = *((uint32_t *)(UID_BASE_ADDR+4));
    uid32[2]= *((uint32_t *)(UID_BASE_ADDR+8));

    return 12;
}

uint8_t crc8_compute(uint8_t* buf,uint8_t len){
    uint8_t crc = 0x00;                       // init = 0

    for(uint8_t i = 0; i < len; i++){
        crc ^= buf[i];                        // 先吃进一个字节
        for(uint8_t j = 0; j < 8; j++){       // 8 次移位
            if(crc & 0x80)                    // 最高位是 1
                crc = (crc << 1) ^ 0x07;      // 左移后异或多项式
            else
                crc = crc << 1;
        }
    }
    return crc;
}


extern "C" {

void ADC1_2_IRQHandler(void){

    //普通
    if(adc_interrupt_flag_get(ADC1, ADC_OCCE_FLAG) != RESET){
        #if 0
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
        #endif
        adc_flag_clear(ADC1, ADC_OCCE_FLAG);
    }
    if(adc_interrupt_flag_get(ADC2, ADC_OCCE_FLAG) != RESET){
        #if 0
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
        #endif
        adc_flag_clear(ADC2, ADC_OCCE_FLAG);
    }

    //抢占
    if(adc_interrupt_flag_get(ADC1, ADC_PCCE_FLAG) != RESET){
        #if 1
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
        #endif
        adc_flag_clear(ADC1, ADC_PCCE_FLAG);
    }

    if(adc_interrupt_flag_get(ADC2, ADC_PCCE_FLAG) != RESET){
        #if 0
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
        #endif
        adc_flag_clear(ADC2, ADC_PCCE_FLAG);
    }

    if(adc_interrupt_flag_get(ADC1, ADC_TCF_FLAG) != RESET){
        adc_flag_clear(ADC1, ADC_TCF_FLAG);
    }

    if(adc_interrupt_flag_get(ADC2, ADC_TCF_FLAG) != RESET){
        adc_flag_clear(ADC2, ADC_TCF_FLAG);
    }
}

void TMR8_OVF_TMR13_IRQHandler(void){
    
    if(tmr_interrupt_flag_get(TMR8,TMR_OVF_FLAG) != RESET){

        #if 0
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
        #endif
        
        _timestamp += TIM1_8_PERIOD * (TIM1_8_RPR + 1);
        if(get_tmr_cnt_dir(TMR8) == CNT_UP_DIR){   //向下计数溢出
            sample();
            NVIC->STIR = Ctrl_Loop_IRQ;
        }else if(get_tmr_cnt_dir(TMR8) == CNT_DW_DIR){  
            //向上计数溢出,要进行PWM输出保护,避免计算太慢导致无法准时更新PWM占空比
            tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, TIM1_8_PERIOD>>1 );
            tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_2, TIM1_8_PERIOD>>1 );
            tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_3, TIM1_8_PERIOD>>1 );

            tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, TIM1_8_PERIOD>>1 );
            tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, TIM1_8_PERIOD>>1 );
            tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, TIM1_8_PERIOD>>1 );
        }
        tmr_flag_clear(TMR8,TMR_OVF_FLAG);
    }
}


void Ctrl_Loop_Handler(){
    #if 0
        if(gpios[CLK_IO].read_output()){
            gpios[CLK_IO].write(0);
        }else{
            gpios[CLK_IO].write(1);
        }
    #endif
        
    std::optional<I_phUVW> current0,current1;

    if(fetch_phase_current(&current0,&current1) == false){
        mt.disarm();
    }
    mt.measure_current(_timestamp - TIM1_CNT_START,current0);


    encoder.update();
    mt.update();
    osc.update();
    
    for(auto& axis:axes){
        axis._VF_openloop_controller.update();
    }
    
    for(auto& axis:axes){
        axis._mt._vf_open_control_law.update(_timestamp);
    }

    for(auto& axis:axes){
        axis._mt._current_control_law.update(_timestamp);
    }

    //硬等
    while(adc_flag_get(ADC2, ADC_OCCE_FLAG) == RESET);
    
    if(fetch_phase_current(&current0,&current1) == false){
        mt.disarm();
    }
    
    mt.dc_cali_update(current0);

    mt.pwm_update(_timestamp + 3*TIM1_8_PERIOD*(TIM1_8_RPR+1) - TIM1_CNT_START);

    for(auto& axis:axes){
        axis.thread_wakeup();
    }
}

}
