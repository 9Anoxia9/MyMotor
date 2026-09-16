#include "crc.h"

void board_crc_init(){
    /* enable crc clock */
    crm_periph_clock_enable(CRM_CRC_PERIPH_CLOCK, TRUE);

    //crc初始值设为0
    crc_init_data_set(0);

    //设置多项式位宽为8bit
    crc_poly_size_set(CRC_POLY_SIZE_8B);

    //设置多项式
    crc_poly_value_set(0x07);
}
