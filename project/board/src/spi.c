#include "spi.h"
#include "gpio.h"


spi_init_type spi_init_struct;


void board_spi3_init(){

    /* master spi initialization */
    crm_periph_clock_enable(CRM_SPI3_PERIPH_CLOCK, TRUE);
    spi_default_para_init(&spi_init_struct);

    //定义SPI3的SCK、MOSI、MISO引脚
    spi3_gpio_init();

    /* dual line unidirectional full-duplex mode */
    spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
    spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_64;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init_struct.frame_bit_num = SPI_FRAME_16BIT;
    spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
    spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
    spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
    spi_init(SPI3, &spi_init_struct);
    
    spi_i2s_interrupt_enable(SPI3, SPI_I2S_RDBF_INT, TRUE);

    spi_enable(SPI3, TRUE);
}

