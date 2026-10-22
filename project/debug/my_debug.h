#ifndef __MY_DEBUG_H__
#define __MY_DEBUG_H__


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "interface_usb.hpp"


#define LOG_WAIT_TICK       5

#ifdef ENABLE_LOG
#define LOG(fmt,...)                        get_log_instance().log(fmt,##__VA_ARGS__)
#define LOG_STACK_SIZE                      (configMINIMAL_STACK_SIZE*2)
#else
#define LOG(fmt,...)  
#define LOG_STACK_SIZE      configMINIMAL_STACK_SIZE
#endif

#if __cplusplus

namespace my_log{

typedef struct log_item_{
    char    buf[128];
    uint8_t len;
}log_item;

class logger {
public:
        logger(){};

        logger& get(){
            return *this;
        }

        void log(const char* fmt, ...);

        void init(Backend* backend,uint8_t que_len);

        static void task(void * ctx);

private:
        QueueHandle_t   _log_que = NULL;
        TaskHandle_t    _log_task = NULL;
        Backend *       _backend = NULL;
};

}

//C++函数
my_log::logger& get_log_instance();
void log_init();


#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_ASSERT
#define ASSERT(expr)                                                          \
    do {                                                                      \
        if (!(expr)) {                                                        \
            for(;;);                                                          \
        }                                                                     \
    } while (0)
#elif defined(ENABLE_ASSERT_PRINT)
#define ASSERT(expr)                                                          \
    do {                                                                      \
        if (!(expr)) {                                                        \
            assert_failed(__FILE__, __LINE__);                                \
        }                                                                     \
    } while (0)    
#else
#define ASSERT(expr) ((void)0)
#endif

void assert_failed(const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif
