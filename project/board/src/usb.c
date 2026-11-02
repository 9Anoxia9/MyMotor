#include "usb.h"

void usb_clock48m_select(usb_clk48_s clk_s)
{
  if(clk_s == USB_CLK_HICK)
  {
    crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_HICK);

    /* enable the acc calibration ready interrupt */
    crm_periph_clock_enable(CRM_ACC_PERIPH_CLOCK, TRUE);

    /* update the c1\c2\c3 value */
    acc_write_c1(7980);
    acc_write_c2(8000);
    acc_write_c3(8020);

    /* open acc calibration */
    acc_calibration_mode_enable(ACC_CAL_HICKTRIM, TRUE);
  }
  else
  {
    crm_pllu_output_set(TRUE);

    crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_PLLU);
  }
}

void board_vbus_sense_init(void)
{
    //忽略VBUS检测，USB直接开始工作
    *(int*)(0x50000038) |= (1<<21);
}

//该函数仅设置时钟USB时钟,初始化靠TinyUSB
void board_usb_init(){

    /* enable usb clock */
    crm_periph_clock_enable(OTG_CLOCK, TRUE);

    /* select usb 48m clcok source */
    usb_clock48m_select(USB_CLK_HICK);
    
    board_vbus_sense_init();
}