#ifndef __GPIO_H__
#define __GPIO_H__

#if __cplusplus
extern "C" {
#endif

#include "at32f45x.h"

//adc引脚
#define M_SO_PORT       GPIOC
#define M_SO_CLK        CRM_GPIOC_PERIPH_CLOCK
#define M0_SO1          GPIO_PINS_0
#define M0_SO2          GPIO_PINS_1
#define M1_SO1          GPIO_PINS_2
#define M1_SO2          GPIO_PINS_3

#define VBUS_S_PORT     GPIOA
#define VBUS_S_CLK      CRM_GPIOA_PERIPH_CLOCK
#define VBUS_S          GPIO_PINS_0
#define AUX_I           GPIO_PINS_6

//pwm引脚
//m0 pwm io
#define M0_MUX          GPIO_MUX_1
#define M0_H_PORT       GPIOA
#define M0_H_CLK        CRM_GPIOA_PERIPH_CLOCK       
#define M0_AH           GPIO_PINS_8
#define M0_BH           GPIO_PINS_9
#define M0_CH           GPIO_PINS_10
#define M0_AH_SRC       GPIO_PINS_SOURCE8
#define M0_BH_SRC       GPIO_PINS_SOURCE9
#define M0_CH_SRC       GPIO_PINS_SOURCE10

#define M0_L_PORT       GPIOB
#define M0_L_CLK        CRM_GPIOB_PERIPH_CLOCK
#define M0_AL           GPIO_PINS_13
#define M0_BL           GPIO_PINS_14
#define M0_CL           GPIO_PINS_15
#define M0_AL_SRC       GPIO_PINS_SOURCE13
#define M0_BL_SRC       GPIO_PINS_SOURCE14
#define M0_CL_SRC       GPIO_PINS_SOURCE15

//m1 pwm io
#define M1_MUX          GPIO_MUX_3
#define M1_H_PORT       GPIOC
#define M1_H_CLK        CRM_GPIOC_PERIPH_CLOCK
#define M1_AH           GPIO_PINS_6
#define M1_BH           GPIO_PINS_7
#define M1_CH           GPIO_PINS_8
#define M1_AH_SRC       GPIO_PINS_SOURCE6
#define M1_BH_SRC       GPIO_PINS_SOURCE7
#define M1_CH_SRC       GPIO_PINS_SOURCE8

#define M1_L_PORT       GPIOB
#define M1_L_CLK        CRM_GPIOB_PERIPH_CLOCK
#define M1_BL           GPIO_PINS_0
#define M1_CL           GPIO_PINS_1
#define M1_BL_SRC       GPIO_PINS_SOURCE0
#define M1_CL_SRC       GPIO_PINS_SOURCE1

#define M1_AL_PORT      GPIOA
#define M1_AL_CLK       CRM_GPIOA_PERIPH_CLOCK
#define M1_AL           GPIO_PINS_7
#define M1_AL_SRC       GPIO_PINS_SOURCE7

//aux pwm io
#define AUX_MUX         GPIO_MUX_9
#define AUX_PORT        GPIOC
#define AUX_CLK         CRM_GPIOC_PERIPH_CLOCK
#define AUX_H           GPIO_PINS_4
#define AUX_L           GPIO_PINS_5
#define AUX_H_SRC       GPIO_PINS_SOURCE4
#define AUX_L_SRC       GPIO_PINS_SOURCE5

//spi io
#define SPI3_MUX         GPIO_MUX_6
#define SPI3_IO_PORT     GPIOC
#define SPI3_IO_CLK      CRM_GPIOC_PERIPH_CLOCK
#define SPI3_SCK         GPIO_PINS_10         
#define SPI3_MISO        GPIO_PINS_11
#define SPI3_MOSI        GPIO_PINS_12
#define SPI3_SCK_SRC     GPIO_PINS_SOURCE10
#define SPI3_MISO_SRC    GPIO_PINS_SOURCE11
#define SPI3_MOSI_SRC    GPIO_PINS_SOURCE12

//can io
#define CAN1_MUX         GPIO_MUX_9
#define CAN1_IO_PORT     GPIOB
#define CAN1_IO_CLK      CRM_GPIOB_PERIPH_CLOCK
#define CAN1_RX          GPIO_PINS_8         
#define CAN1_TX          GPIO_PINS_9
#define CAN1_RX_SRC      GPIO_PINS_SOURCE8
#define CAN1_TX_SRC      GPIO_PINS_SOURCE9

void adc_gpio_init(void);
void m0_pwm_gpio_init(void);
void m1_pwm_gpio_init(void);
void aux_pwm_gpio_init(void);
void spi3_gpio_init(void);
void can_gpio_init(void);


#if __cplusplus
}
#endif

#endif