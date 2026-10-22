#include <stdio.h>
#include <stdarg.h>
#include "my_debug.h"
#include "interface_usb.hpp"
#include "interface_usart.hpp"


my_log::logger log_instance;

void my_log::logger::log(const char* fmt,...){

    log_item item;

    va_list args;
    va_start(args,fmt);

    //vsnprinf格式化字符串,并返回格式化后的字符串长度无论是否发生截断
    item.len = vsnprintf(item.buf,sizeof(item.buf),fmt,args);
    if( item.len > static_cast<uint8_t>(sizeof(item.buf)) ){
        strcpy(item.buf,"too long!\n");
        //strlen检测到第一个'\0'时就会输出长度
        item.len = strlen(item.buf);
    }

    xQueueSend(this->_log_que,&item,LOG_WAIT_TICK); //就算超期也啥都不做，让它吧。
    
    va_end(args);
}

void my_log::logger::task(void * ctx){

    log_item item;
    logger* instance = static_cast<logger*>(ctx);
    UBaseType_t stack_use_size = 0;

    while(1){
        if(xQueueReceive(instance->_log_que,&item,portMAX_DELAY) == pdPASS){
            if(instance->_backend->connected()){
                if(instance->_backend->write(item.buf,item.len)){
                    instance->_backend->flush();   
                }
            }
        }
        stack_use_size = uxTaskGetStackHighWaterMark(NULL);
    }
    (void)stack_use_size;
}

void my_log::logger::init(Backend * backend,uint8_t que_len){

    this->_backend = backend;
    backend->init();
    this->_log_que =  xQueueCreate(que_len,sizeof(log_item));
    xTaskCreate(this->task,"log",CDC_STACK_SIZE,this,1,\
                    &(this->_log_task));
    
}

void log_init(){
#ifdef ENABLE_USB_LOG
    log_instance.init(&usb_logbackend,10);
#else
    log_instance.init(&usart_logbackend,10);
#endif
}

my_log::logger& get_log_instance(){
    return log_instance;
}

// TinyUSB 调试输出包装函数,桥接到本项目的 log 系统(异步队列,最终走 UsartBackend)
extern "C" int tusb_debug_printf(const char* fmt, ...){

    char buf[128];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    get_log_instance().log("%s", buf);

    return len;
}
