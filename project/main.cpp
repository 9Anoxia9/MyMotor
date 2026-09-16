#include "main.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "my_debug.h"
#include "protocol.hpp"


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

    /*for (auto& axis: axes){
        axis.run_open_loop();
    }*/
}

extern "C" int main(void) {
    
    system_init();

    board_init();

    tinyusb_init();

    log_init();

    my_prot_init();

    motor_ctrl_init();

    start_timer();

    uint32_t heap_free = xPortGetFreeHeapSize();
    (void)heap_free; //堆的空闲字节
    
    //创建IdleTask和TimerTask以及TimerQueue一共要1224个字节的heap
    vTaskStartScheduler();    

    while (1)
    {
        ;
    }  
}