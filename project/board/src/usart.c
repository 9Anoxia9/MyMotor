#include "usart.h"
#include "gpio.h"


static void usart2_dma_init(void){

    dma_init_type dma_init_struct;

    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
    dmamux_enable(DMA1, TRUE);

    /* DMA1_CHANNEL1: USART2 TX, memory 地址由 usart2_dma_send 动态设置 */
    dma_reset(DMA1_CHANNEL1);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = 0;
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = 0;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART2->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL1, &dma_init_struct);

    dmamux_init(DMA1MUX_CHANNEL1, DMAMUX_DMAREQ_ID_USART2_TX);

    /* 使能 TX 完成中断,发送完成后由 interface_usart.cpp 的中断处理 give 信号量 */
    dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, TRUE);
    nvic_irq_enable(DMA1_Channel1_IRQn, 6, 0);

    /* DMA1_CHANNEL2: USART2 RX, memory 地址后续由用户提供 buffer 后设置 */
    dma_reset(DMA1_CHANNEL2);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = 0;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = 0;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART2->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL2, &dma_init_struct);

    dmamux_init(DMA1MUX_CHANNEL2, DMAMUX_DMAREQ_ID_USART2_RX);
}

void board_usart2_init(void){

    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);

    usart2_gpio_init();

    usart_init(USART2, USART2_BAUDRATE, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART2, TRUE);
    usart_receiver_enable(USART2, TRUE);
    usart_dma_transmitter_enable(USART2, TRUE);
    usart_dma_receiver_enable(USART2, TRUE);
    usart_enable(USART2, TRUE);

    usart2_dma_init();
}

uint16_t usart2_dma_send(uint8_t* data, uint16_t len){

    if(len == 0){
        return 0;
    }

    dma_channel_enable(DMA1_CHANNEL1, FALSE);

    DMA1_CHANNEL1->maddr = (uint32_t)data;
    dma_data_number_set(DMA1_CHANNEL1, len);
    dma_channel_enable(DMA1_CHANNEL1, TRUE);

    return len;
}
