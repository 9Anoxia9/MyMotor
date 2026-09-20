#include "main.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "my_debug.h"
#include "protocol.hpp"


TaskHandle_t main_task_handle;

extern "C"{

void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{
    for(;;);
}

}

void sample(){
    encoder.abs_spi_sample();
}

void motor_ctrl_init(){
    encoder.init();

    for (auto& axis: axes){
        axis.init();
    }
}

void main_task(void* ctx){

    motor_ctrl_init();

    start_timer();

    vTaskDelete(NULL);
}

extern "C" int main(void) {
    
    system_init();

    board_init();

    tinyusb_init();

    log_init();

    my_protocol_init();

    motor_ctrl_init();

    start_timer();

    //xTaskCreate(main_task,"main",LOG_STACK_SIZE,NULL,1,&main_task_handle);

    uint32_t heap_free = xPortGetFreeHeapSize();
    (void)heap_free; //堆的空闲字节
    
    //创建IdleTask和TimerTask以及TimerQueue一共要1224个字节的heap
    vTaskStartScheduler();    

    while (1)
    {
        ;
    }  
}