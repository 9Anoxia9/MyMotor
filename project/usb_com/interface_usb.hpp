#ifndef __INTERFACE_USB_HPP__
#define __INTERFACE_USB_HPP__

#include "tusb.h"
#include "tusb_config.h"
#include "common.hpp"


#define CDC_LOG_ITF        0
#define CDC_BIT_ITF        1


#define USBD_STACK_SIZE    (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 4 : 2))
#define CDC_STACK_SIZE     (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 3 : 2))

#if __cplusplus
class UsbCdcBackend : public Backend{
public:
        UsbCdcBackend(uint8_t itf): _itf(itf){}

        bool connected(){
          return tud_cdc_connected(_itf);
        }

        uint32_t available(){
          return tud_cdc_available(_itf);
        }

        uint32_t read(void* buffer, uint32_t bufsize){
          return tud_cdc_read(_itf,buffer,bufsize);
        }

        uint32_t write(void const* buf, uint32_t size){
          return tud_cdc_write(_itf,buf, size);
        }

        uint32_t flush(){
          return tud_cdc_write_flush(_itf);
        }
private:
        uint8_t _itf;
};


extern UsbCdcBackend usb_logbackend;
extern UsbCdcBackend usb_protocol_backend;

#endif


#if __cplusplus
extern "C" {
#endif

size_t usb_get_serial(uint16_t desc_str1[], size_t max_chars);
void tinyusb_init(void);
void usb_device_task(void *param);
void led_fun(void * ctx);

#if __cplusplus
}
#endif

#endif