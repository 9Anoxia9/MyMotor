#include "crm.h"

#if OUTPUT_SCLK_FRQ == 1
void clkout_config(void)
{
  gpio_init_type gpio_init_struct;

  /* enable gpio port clock */
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* clkout gpio init */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOC, &gpio_init_struct);  //复用寄存器上电初始化默认为MUX0

  /* config clkout division */
  crm_clkout_div_set(CRM_CLKOUT_DIV1_4, CRM_CLKOUT_DIV2_1); //48MHz

  /* config clkout clock */
  crm_clock_out_set(CRM_CLKOUT_SCLK);
}
#endif

void system_clock_config_192mhz(void)
{
  uint16_t pll_ns, pll_ms, pll_fp;

  /* reset crm */
  crm_reset();

  /* config flash psr register */
  flash_psr_set(FLASH_WAIT_CYCLE_5);

  /* enable pwc periph clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* set power ldo output voltage to 1.3v */
  pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

  crm_clock_source_enable(CRM_CLOCK_SOURCE_HICK, TRUE);

  /* calculate pll parameter according to the function */
  crm_pll_parameter_calculate(CRM_PLL_SOURCE_HICK, SYSCLK_FRQ, &pll_ms, &pll_ns, &pll_fp);

  /* config pll clock source */
  crm_pll_config(CRM_PLL_SOURCE_HICK, pll_ns, pll_ms, (crm_pll_fp_type)pll_fp);

  /* enable pll */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);

  /* wait till pll is ready */
  while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
  {
  }

  /* config ahbclk */
  crm_ahb_div_set(CRM_AHB_DIV_1);

  /* config apb3clk, the maximum frequency of APB3 clock is 90 MHz */
  crm_apb3_div_set(CRM_APB3_DIV_4); //48MHz

  /* config apb2clk, the maximum frequency of APB2 clock is 192 MHz */
  crm_apb2_div_set(CRM_APB2_DIV_1); //192MHz

  /* config apb1clk, the maximum frequency of APB1 clock is 192 MHz */
  crm_apb1_div_set(CRM_APB1_DIV_1); //192MHz

  /* enable auto step mode */
  crm_auto_step_mode_enable(TRUE);

  /* select pll as system clock source */
  crm_sysclk_switch(CRM_SCLK_PLL);

  /* wait till pll is used as system clock source */
  while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
  {
  }

  /* disable auto step mode */
  crm_auto_step_mode_enable(FALSE);

  /* update system_core_clock global variable */
  system_core_clock_update();
}

void system_clock_init(void)
{
  system_clock_config_192mhz();

#if OUTPUT_SCLK_FRQ == 1
  clkout_config();              //输出SCLK的四分频
#endif
}
