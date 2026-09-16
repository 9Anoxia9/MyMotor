#include "at32_spi.hpp"
#include "board.h"


Spi spi3_intf(SPI3);

void Spi::start(SpiTask *task){
    if(task == nullptr){
        return;
    }
    
    SpiTask** ptr = &_task_list;
    while(*ptr){ptr = &((*ptr)->next);}
    *ptr = task;
    task->used = true;

    if(task == _task_list){
        tansfer();
    }
}

void Spi::tansfer(){
    uint8_t delay = 15; //1.33us
    _task_list->ncs_gpio.write(false);
    while(delay--);
    spi_i2s_data_transmit(_hspi,_task_list->tx_data);
}

void Spi::finish(){

    if(_task_list == nullptr){
        return;
    }

    _task_list->ncs_gpio.write(true);
    *(_task_list->rx_data) = spi_i2s_data_receive(_hspi);

    if(_task_list->finish != nullptr){
        _task_list->finish(_task_list->finish_ctx,true);
    }
    
    release_task();
    if(_task_list != nullptr){
        tansfer();
    }
}


extern "C" {

void SPI3_I2S3EXT_IRQHandler(){
    if(spi_i2s_interrupt_flag_get(SPI3, SPI_I2S_RDBF_FLAG) != RESET)
    {
        spi3_intf.finish();
    }
}

}
