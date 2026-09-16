#include "adc.h"
#include "gpio.h"

void board_adc_init(){

    adc_common_config_type adc_common_struct;
    adc_base_config_type adc_base_struct;

    crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_ADC2_PERIPH_CLOCK, TRUE);

    adc_reset();

    adc_common_default_para_init(&adc_common_struct);

    /* config combine mode */
    adc_common_struct.combine_mode = ADC_INDEPENDENT_MODE;

    /* config division,adcclk is division by hclk */
    adc_common_struct.div = ADC_HCLK_DIV_3; //48MHz

    /* config common dma mode,it's not useful in independent mode */
    adc_common_struct.common_dma_mode = ADC_COMMON_DMAMODE_DISABLE;

    /* config common dma request repeat */
    adc_common_struct.common_dma_request_repeat_state = FALSE;

    /* config adjacent adc sampling interval,it's useful for ordinary shifting mode */
    adc_common_struct.sampling_interval = ADC_SAMPLING_INTERVAL_5CYCLES;

    /* config inner temperature sensor and vintrv */
    adc_common_struct.tempervintrv_state = FALSE;

    adc_common_struct.vbat_state = FALSE;
    adc_common_config(&adc_common_struct);

    adc_base_default_para_init(&adc_base_struct);

    adc_base_struct.sequence_mode = TRUE;
    adc_base_struct.repeat_mode = FALSE;
    adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
    adc_base_struct.ordinary_channel_length = 1;
    adc_base_config(ADC1, &adc_base_struct);
    adc_resolution_set(ADC1, ADC_RESOLUTION_12B);

    adc_base_config(ADC2, &adc_base_struct);
    adc_resolution_set(ADC2, ADC_RESOLUTION_12B);

    /* config ordinary channel */
    //M1_SO1,快速通道,15个Cycle,312.5ns
    adc_ordinary_channel_set(ADC1, ADC_CHANNEL_12, 1, ADC_SAMPLETIME_2_5);  //M1_SO1,普通序列1
    adc_ordinary_channel_set(ADC2, ADC_CHANNEL_13, 1, ADC_SAMPLETIME_2_5);  //M1_SO2,普通序列1

    /*TIM8触发ADC1、ADC2的普通通道转换*/
    adc_ordinary_conversion_trigger_set(ADC1, ADC_ORDINARY_TRIG_TMR8TRGOUT, ADC_ORDINARY_TRIG_EDGE_RISING);
    adc_ordinary_conversion_trigger_set(ADC2, ADC_ORDINARY_TRIG_TMR8TRGOUT, ADC_ORDINARY_TRIG_EDGE_RISING);

    /* config dma mode,it's not useful when common dma mode is use */
    adc_dma_mode_enable(ADC1, FALSE);
    adc_dma_mode_enable(ADC2, FALSE);

    /* config dma request repeat,it's not useful when common dma mode is use */
    adc_dma_request_repeat_enable(ADC1, FALSE);
    adc_dma_request_repeat_enable(ADC2, FALSE);

    /* config preempt channel */
    adc_preempt_channel_length_set(ADC1, 2);
    adc_preempt_channel_length_set(ADC2, 2);
    adc_preempt_channel_set(ADC1, ADC_CHANNEL_10, 1, ADC_SAMPLETIME_2_5);   //M0_SO1,抢占序列3
    adc_preempt_channel_set(ADC1, ADC_CHANNEL_0, 2, ADC_SAMPLETIME_2_5);    //VBUS,抢占序列4
    adc_preempt_channel_set(ADC2, ADC_CHANNEL_11, 1, ADC_SAMPLETIME_2_5);   //M0_SO2,抢占序列3
    adc_preempt_channel_set(ADC2, ADC_CHANNEL_6, 2, ADC_SAMPLETIME_6_5);    //AUX_I,抢占序列4(慢速通道)

    /*TIM1触发ADC1、ADC2抢占通道组转换*/
    adc_preempt_conversion_trigger_set(ADC1, ADC_PREEMPT_TRIG_TMR1TRGOUT, ADC_PREEMPT_TRIG_EDGE_RISING);
    adc_preempt_conversion_trigger_set(ADC2, ADC_PREEMPT_TRIG_TMR1TRGOUT, ADC_PREEMPT_TRIG_EDGE_RISING);

    #if 0
    nvic_irq_enable(ADC1_2_IRQn, 1, 0);

    adc_interrupt_enable(ADC1, ADC_OCCE_INT, TRUE);
    adc_interrupt_enable(ADC2, ADC_OCCE_INT, TRUE);

    /* enable adc preempt channels conversion end interrupt */
    adc_interrupt_enable(ADC1, ADC_PCCE_INT, TRUE);
    adc_interrupt_enable(ADC2, ADC_PCCE_INT, TRUE);
    #endif

    adc_gpio_init();    

    /* adc enable */
    adc_enable(ADC1, TRUE);
    adc_enable(ADC2, TRUE);
    while(adc_flag_get(ADC1, ADC_RDY_FLAG) == RESET && adc_flag_get(ADC2, ADC_RDY_FLAG) == RESET);

    /* adc calibration */
    adc_calibration_init(ADC1);
    adc_calibration_init(ADC2);
    while(adc_calibration_init_status_get(ADC1) && adc_calibration_init_status_get(ADC2));
    adc_calibration_start(ADC1);
    adc_calibration_start(ADC2);
    while(adc_calibration_status_get(ADC1) && adc_calibration_status_get(ADC2));
}