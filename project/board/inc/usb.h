#ifndef __USB_H__
#define __USB_H__

#if __cplusplus
extern "C" {
#endif

#include "at32f45x.h"

#define OTG_CLOCK               CRM_OTGFS1_PERIPH_CLOCK
#define OTG_CLK_SRC             USB_CLK_HICK
#define OTG_IRQ                 OTGFS1_IRQn
#define OTG_IRQ_HANDLER         OTGFS1_IRQHandler
#define OTG_WKUP_IRQ            OTGFS1_WKUP_IRQn
#define OTG_WKUP_HANDLER        OTGFS1_WKUP_IRQHandler


void board_usb_init();

#if __cplusplus
}
#endif


#endif

