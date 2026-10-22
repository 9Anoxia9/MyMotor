#include "gpio.h"

void adc_gpio_init(void){
    
    gpio_init_type gpio_initstructure;

    crm_periph_clock_enable(M_SO_CLK, TRUE);
    crm_periph_clock_enable(VBUS_S_CLK, TRUE);

    gpio_default_para_init(&gpio_initstructure);

    /* config adc pin as analog input mode */
    gpio_initstructure.gpio_mode = GPIO_MODE_ANALOG;
    gpio_initstructure.gpio_pins = M0_SO1 |M0_SO2 |M1_SO1|M1_SO2;
    gpio_init(M_SO_PORT, &gpio_initstructure);

    gpio_initstructure.gpio_pins = VBUS_S | AUX_I;
    gpio_init(VBUS_S_PORT, &gpio_initstructure);
}

void m0_pwm_gpio_init(void){
    
    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(M0_H_CLK, TRUE);
    crm_periph_clock_enable(M0_L_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = M0_AH | M0_BH | M0_CH;
    gpio_init(M0_H_PORT, &gpio_init_struct);

    gpio_init_struct.gpio_pins = M0_AL | M0_BL | M0_CL;
    gpio_init(M0_L_PORT, &gpio_init_struct);

    gpio_pin_mux_config(M0_H_PORT, M0_AH_SRC, M0_MUX);
    gpio_pin_mux_config(M0_H_PORT, M0_BH_SRC, M0_MUX);
    gpio_pin_mux_config(M0_H_PORT, M0_CH_SRC, M0_MUX);

    gpio_pin_mux_config(M0_L_PORT, M0_AL_SRC, M0_MUX);
    gpio_pin_mux_config(M0_L_PORT, M0_BL_SRC, M0_MUX);
    gpio_pin_mux_config(M0_L_PORT, M0_CL_SRC, M0_MUX);
}

void m1_pwm_gpio_init(void){

    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(M1_H_CLK, TRUE);
    crm_periph_clock_enable(M1_L_CLK, TRUE);
    crm_periph_clock_enable(M1_AL_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = M1_AH | M1_BH | M1_CH;
    gpio_init(M1_H_PORT, &gpio_init_struct);

    gpio_init_struct.gpio_pins = M1_BL | M1_CL;
    gpio_init(M1_L_PORT, &gpio_init_struct);

    gpio_init_struct.gpio_pins = M1_AL;
    gpio_init(M1_AL_PORT, &gpio_init_struct);

    gpio_pin_mux_config(M1_H_PORT, M1_AH_SRC, M1_MUX);
    gpio_pin_mux_config(M1_H_PORT, M1_BH_SRC, M1_MUX);
    gpio_pin_mux_config(M1_H_PORT, M1_CH_SRC, M1_MUX);

    gpio_pin_mux_config(M1_AL_PORT, M1_AL_SRC, M1_MUX);
    gpio_pin_mux_config(M1_L_PORT, M1_BL_SRC, M1_MUX);
    gpio_pin_mux_config(M1_L_PORT, M1_CL_SRC, M1_MUX);
}

void aux_pwm_gpio_init(void){

    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(AUX_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = AUX_H | AUX_L;
    gpio_init(AUX_PORT, &gpio_init_struct);

    gpio_pin_mux_config(AUX_PORT, AUX_H_SRC, AUX_MUX);
    gpio_pin_mux_config(AUX_PORT, AUX_L_SRC, AUX_MUX);
}

void spi3_gpio_init(void){
    
    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(SPI3_IO_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    //spi sck io
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = SPI3_SCK;
    gpio_init(SPI3_IO_PORT, &gpio_init_struct);

    //spi mosi io
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_pins = SPI3_MOSI;
    gpio_init(SPI3_IO_PORT, &gpio_init_struct);

    //spi miso io
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init_struct.gpio_pins = SPI3_MISO;
    gpio_init(SPI3_IO_PORT, &gpio_init_struct);

    gpio_pin_mux_config(SPI3_IO_PORT, SPI3_SCK_SRC, SPI3_MUX);
    gpio_pin_mux_config(SPI3_IO_PORT, SPI3_MOSI_SRC, SPI3_MUX);
    gpio_pin_mux_config(SPI3_IO_PORT, SPI3_MISO_SRC, SPI3_MUX);
}

void can_gpio_init(void){

    gpio_init_type gpio_init_struct;

    /* enable the gpio clock */
    crm_periph_clock_enable(CAN1_IO_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    /* configure the can tx, rx, stb pin */
    /* can_stb connect to the transceiver <stby> pin, default output low, keep the transceiver
    running in normal mode */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = CAN1_RX | CAN1_TX;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(CAN1_IO_PORT, &gpio_init_struct);

    gpio_pin_mux_config(CAN1_IO_PORT, CAN1_RX_SRC, CAN1_MUX);
    gpio_pin_mux_config(CAN1_IO_PORT, CAN1_TX_SRC, CAN1_MUX);
}

void usart2_gpio_init(void){

    gpio_init_type gpio_init_struct;

    crm_periph_clock_enable(USART2_IO_CLK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pins = USART2_TX | USART2_RX;
    gpio_init(USART2_IO_PORT, &gpio_init_struct);

    gpio_pin_mux_config(USART2_IO_PORT, USART2_TX_SRC, USART2_MUX);
    gpio_pin_mux_config(USART2_IO_PORT, USART2_RX_SRC, USART2_MUX);
}
