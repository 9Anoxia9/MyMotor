#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>
#include <stddef.h>
#include "FreeRTOSConfig.h"
#include "crm.h"
#include "tim.h"
#include "adc.h"
#include "usb.h"
#include "spi.h"


#define MAX_VOLTAGE         24.0f
#define MOTOR_CURRENT_RES   0.005f

#define AXIS_COUNT          1

#define CTRL_DT             (2.0f*(TIM1_8_RPR+1)*TIM1_8_PERIOD/TIM1_8_SRC_FRQ)


//器件唯一标识符寄存器地址,用于USB ID
#define UID_BASE_ADDR   0x1FFFF7E8

#define GPIO_COUNT 8

enum gpio_mode{
    GPIO_INPUT_PULL_NONE,
    GPIO_INPUT_PULL_UP,
    GPIO_INPUT_PULL_DOWN,
    GPIO_OUTPUT_PP,
    GPIO_OUTPUT_OD_PULL_NONE,
    GPIO_OUTPUT_OD_PULL_UP,
    GPIO_ANALOG
};

enum gpio_idx{
    LED2 = 0,
    GPIO1,
    GPIO2,
    GPIO3,
    GPIO4,
    GPIO5,
    CLK_IO,
    M1_ENC_Z,
    M0_ENC_Z,
    ALL_CONFIGABLE_GPIO
};

#define GPIO_DEFAULT_CONFIG {   \
    GPIO_OUTPUT_PP,         \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_OUTPUT_PP,         \
    GPIO_INPUT_PULL_DOWN,   \
    GPIO_INPUT_PULL_DOWN    \
}


#ifdef __cplusplus
#include "at32_gpio.hpp"
#include "at32_spi.hpp"
#include "oscilloscope.hpp"
#include "axis.hpp"


#define CRITICAL_SECTION  if(CriticalSection critical_section{})


static inline uint32_t enter_critical(){
    uint32_t basepri = __get_BASEPRI();

    return basepri;
}

static inline void exit_critical(uint32_t basepri){
    __set_BASEPRI(basepri);
}

struct CriticalSection
{
   CriticalSection(const CriticalSection&) = delete;
   CriticalSection(const CriticalSection&&) = delete;
   void operator=(const CriticalSection&) = delete;
   void operator=(const CriticalSection&&) = delete;
   operator bool() {return true;}
   CriticalSection(){_mask = enter_critical();}
   ~CriticalSection(){exit_critical(_mask);}
   uint32_t _mask;
};



extern Motor mt;
extern Encoder encoder;
extern std::array<Axis,AXIS_COUNT> axes;

extern Osc  osc;
extern Gpio gpios[]; 

inline static const Gpio get_gpio(gpio_idx idx){
    return (idx < ALL_CONFIGABLE_GPIO) ? gpios[idx] : Gpio::none;
}

#endif

void system_init(void);
void board_init(void);
void start_timer();
size_t board_get_unique_id(uint8_t id[], size_t max_len);
uint8_t crc8_compute(uint8_t* buf,uint8_t len);


#endif
