#ifndef __AT32_SPI_HPP__
#define __AT32_SPI_HPP__


#include "at32_gpio.hpp"


#if __cplusplus

class Spi {
public:
    struct SpiTask{
        Gpio ncs_gpio;
        uint16_t tx_data;
        uint16_t *rx_data;
        void (*finish)(void *,bool);
        void* finish_ctx;
        bool used;
        SpiTask* next;
    };


    Spi(spi_type* hspi): _hspi(hspi){}

    spi_type* operator*() {return this->_hspi;}


    bool check_task_is_vaild(SpiTask* task) {
        return task->used == false;
    }

    void release_task() {
        if(_task_list->next != nullptr){
            _task_list = _task_list->next;
        }else{
            _task_list = nullptr;
        }
    }

    void start(SpiTask *task);

    void tansfer();

    void finish();


private:
    spi_type*       _hspi = nullptr;
    SpiTask*        _task_list = nullptr;
};


extern Spi spi3_intf;

#endif

#endif