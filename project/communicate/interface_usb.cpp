#include "FreeRTOS.h"
#include "task.h"
#include "interface_usb.hpp"
#include "board.h"
#include "protocol.hpp"


enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

TaskHandle_t USBHandler = NULL;
TaskHandle_t LedHandler = NULL;

#ifdef ENABLE_USB_LOG
UsbCdcBackend usb_logbackend(CDC_LOG_ITF);
#endif

UsbCdcBackend usb_protocol_backend(CDC_BIT_ITF);


void tinyusb_init(void){

  xTaskCreate(usb_device_task,"usbd",USBD_STACK_SIZE,NULL,configMAX_PRIORITIES - 1, &USBHandler);
  xTaskCreate(led_fun,"led",configMINIMAL_STACK_SIZE,NULL,1,&LedHandler);
}

size_t usb_get_serial(uint16_t desc_str1[], size_t max_chars) {
  uint8_t uid[16] TU_ATTR_ALIGNED(4);
  size_t uid_len;

  // TODO work with make, but not working with esp32s3 cmake
  uid_len = board_get_unique_id(uid, sizeof(uid));

  if ( uid_len > max_chars / 2u ) {
    uid_len = max_chars / 2u;
  }

  for ( size_t i = 0; i < uid_len; i++ ) {
    for ( size_t j = 0; j < 2; j++ ) {
      const unsigned char nibble_to_hex[16] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
      };
      const uint8_t nibble = (uint8_t) ((uid[i] >> (j * 4u)) & 0xfu);
      desc_str1[i * 2 + (1 - j)] = nibble_to_hex[nibble]; // UTF-16-LE
    }
  }

  return 2 * uid_len;
}

void usb_device_task(void *param) {
  (void) param;

    tusb_rhport_init_t dev_init = {
    .role = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();
  }
}

void led_fun(void * ctx)
{
    Gpio led = get_gpio(LED2);
    while (1)
    {
        vTaskDelay(blink_interval_ms);
        if(led.read_output() == SET){
            led.write(0);
        }else{
            led.write(1);
        }
    }
}

extern "C" {

void tud_cdc_rx_cb(uint8_t itf){
  if(itf == CDC_BIT_ITF){
    Protocol& p = get_my_prot();
    xSemaphoreGive(p._sem);
  }
}

// Invoked when device is mounted
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

void OTGFS1_IRQHandler(void) {
  tusb_int_handler(0, true);
}
void OTGFS1_WKUP_IRQHandler(void) {
  tusb_int_handler(0, true);
}

}





