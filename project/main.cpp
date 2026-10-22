#include "main.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "my_debug.h"
#include "protocol.hpp"


TaskHandle_t main_task_handle;

extern "C"{

void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{
    UBaseType_t stack_use_size = 0;
    stack_use_size = uxTaskGetStackHighWaterMark(NULL);
    for(;;);
    (void)stack_use_size;
}

}

void sample(){
    encoder.abs_spi_sample();
}

void motor_ctrl_init(){
    encoder.init();

    control.init();

    for (auto& axis: axes){
        axis.init();
    }
}

void main_task(void* ctx){

    motor_ctrl_init();

    start_timer();

    vTaskDelete(NULL);
}

void period_print(void* ctx){
    while(1){
        LOG("Hello World!\n");
        vTaskDelay(1000);
    }
}

extern "C" int main(void) {
    
    system_init();

    board_init();

    tinyusb_init();

    log_init();

    my_protocol_init();

    motor_ctrl_init();

    start_timer();

    xTaskCreate(period_print,"period_log",configMINIMAL_STACK_SIZE*2,NULL,configMAX_PRIORITIES - 1,NULL);

    uint32_t heap_free = xPortGetFreeHeapSize();
    (void)heap_free; //堆的空闲字节
    
    //创建IdleTask和TimerTask以及TimerQueue一共要1224个字节的heap
    vTaskStartScheduler();    

    while (1)
    {
        ;
    }  
}