#include "interface_usart.hpp"
#include "board.h"

UsartBackend usart_logbackend;

extern "C" {

void DMA1_Channel1_IRQHandler(void){

    if(dma_interrupt_flag_get(DMA1_FDT1_FLAG)){
        dma_flag_clear(DMA1_FDT1_FLAG);
        usart_logbackend.notify_tx_done_from_isr();
    }
}

}
