#ifndef __USART_H__
#define __USART_H__

#if __cplusplus
extern "C" {
#endif

#include "at32f45x.h"

#define USART2_BAUDRATE     115200

void board_usart2_init(void);
uint16_t usart2_dma_send(uint8_t* data, uint16_t len);

#if __cplusplus
}
#endif

#endif
