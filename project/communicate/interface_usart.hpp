#ifndef __INTERFACE_USART_HPP__
#define __INTERFACE_USART_HPP__

#include "common.hpp"
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"

#if __cplusplus
class UsartBackend : public Backend{
public:
    UsartBackend(){}

    void init() override {
        _tx_sem = xSemaphoreCreateBinary();
    }

    bool connected(){
        return true;    //串口无连接概念,总是可用
    }

    uint32_t available(){
        return 0;       //暂不支持接收
    }

    uint32_t read(void* buffer, uint32_t bufsize){
        return 0;
    }

    uint32_t write(void const* buf, uint32_t size){
        size = usart2_dma_send((uint8_t*)buf, size);
        return size;
    }

    uint32_t flush(){
        if(_tx_sem != NULL){
            xSemaphoreTake(_tx_sem, portMAX_DELAY);
        }
        return 0;
    }

    // 供 DMA 完成中断(ISR 上下文)调用
    void notify_tx_done_from_isr(){
        if(_tx_sem != NULL){
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(_tx_sem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

private:
    SemaphoreHandle_t _tx_sem = NULL;
};

extern UsartBackend usart_logbackend;

#endif

#endif
