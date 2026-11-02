#include "at32_gpio.hpp"


#define EXTI_NUM    16
static struct exint_cb {
    void (*cb)(void*);
    void *ctx;
}EXTI_CB[EXTI_NUM];


const Gpio Gpio::none{nullptr, 0};

void Gpio::port_clken(){

    ASSERT(port_ != nullptr);

    if(port_ == GPIOA){
        crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    }else if(port_ == GPIOB){
        crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    }else if(port_ == GPIOC){
        crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
    }else if(port_ == GPIOD){
        crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
    }else if(port_ == GPIOH){
        crm_periph_clock_enable(CRM_GPIOH_PERIPH_CLOCK, TRUE);
    }
}

bool Gpio::set_output(gpio_output_type type,gpio_pull_type pull){
    
    ASSERT(pull <= GPIO_PULL_DOWN && type <= GPIO_OUTPUT_OPEN_DRAIN);

    gpio_init_type gpio_init_struct;
    
    port_clken();

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_out_type = type;
    gpio_init_struct.gpio_pull = pull;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = pin_;
    
    gpio_init(port_, &gpio_init_struct);

    return true;
}

bool Gpio::set_input(gpio_pull_type pull){
    
    ASSERT(pull <= GPIO_PULL_DOWN);

    gpio_init_type gpio_init_struct;
    
    port_clken();

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_pull = pull;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = pin_;
    
    gpio_init(port_, &gpio_init_struct);

    return true;
}

bool Gpio::set_analog(){

    ASSERT(port_ != nullptr);

    gpio_init_type gpio_init_struct;
    
    port_clken();

    gpio_default_para_init(&gpio_init_struct);

    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = pin_;
    
    gpio_init(port_, &gpio_init_struct);

    return true;
}

bool Gpio::set_exint(exint_polarity_config_type polarity,
                        uint32_t preempt_priority,uint32_t sub_priority,
                        void (*cb)(void*),void* ctx){

    exint_init_type exint_init_struct;
    scfg_port_source_type port_src;

    ASSERT( port_ != nullptr && polarity <= EXINT_TRIGGER_BOTH_EDGE && cb != nullptr);

    crm_periph_clock_enable(CRM_SCFG_PERIPH_CLOCK, TRUE);
    if(port_ == GPIOA){
        crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
        port_src = SCFG_PORT_SOURCE_GPIOA;
    }else if(port_ == GPIOB){
        crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
        port_src = SCFG_PORT_SOURCE_GPIOB;
    }else if(port_ == GPIOC){
        crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
        port_src = SCFG_PORT_SOURCE_GPIOC;
    }else if(port_ == GPIOD){
        crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
        port_src = SCFG_PORT_SOURCE_GPIOD;
    }else if(port_ == GPIOH){
        crm_periph_clock_enable(CRM_GPIOH_PERIPH_CLOCK, TRUE);
        port_src = SCFG_PORT_SOURCE_GPIOH;
    }else{
        ASSERT(false);
    }

    scfg_exint_line_config(port_src, get_pin_source());

    exint_default_para_init(&exint_init_struct);
    exint_init_struct.line_enable = TRUE;
    exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
    exint_init_struct.line_select = static_cast<uint32_t>(pin_);
    exint_init_struct.line_polarity = polarity;
    exint_init(&exint_init_struct);

    nvic_irq_enable(get_exint_iqr_number(), preempt_priority, sub_priority);

    EXTI_CB[get_pin_number()].cb = cb;
    EXTI_CB[get_pin_number()].ctx = ctx;

    return true;
}

bool Gpio::reset_exint(){

    exint_init_type exint_init_struct;

    ASSERT(port_ != nullptr);

    exint_default_para_init(&exint_init_struct);
    exint_init_struct.line_select = static_cast<uint32_t>(pin_);
    exint_init(&exint_init_struct);

    EXTI_CB[get_pin_number()].cb = nullptr;
    EXTI_CB[get_pin_number()].ctx = nullptr;

    return true;
}

IRQn_Type Gpio::get_exint_iqr_number() const {

    uint32_t pin_number = get_pin_number();

    ASSERT(pin_number <= 15);

    switch(pin_number){
        case 0: return EXINT0_IRQn;
        case 1: return EXINT1_IRQn;
        case 2: return EXINT2_IRQn;
        case 3: return EXINT3_IRQn;
        case 4: return EXINT4_IRQn;
        case 5: 
        case 6:
        case 7:
        case 8:
        case 9: return EXINT9_5_IRQn;
        case 10: 
        case 11:
        case 12:
        case 13:
        case 14: 
        case 15: return EXINT15_10_IRQn;
        default:
                 __builtin_unreachable();   //此处不可达
    }
}

static void exti_handle(uint32_t idx){

    uint32_t line = (1<<idx);

    if(exint_interrupt_flag_get(line) == RESET)
    {
        return;
    }

    exint_flag_clear(line);

    if(EXTI_CB[idx].cb != nullptr){
        EXTI_CB[idx].cb(EXTI_CB[idx].ctx);
    }
}

extern "C" {

void EXINT0_IRQHandler(void){
    exti_handle(0);
}

void EXINT1_IRQHandler(void){
    exti_handle(1);
}

void EXINT2_IRQHandler(void){
    exti_handle(2);
}

void EXINT3_IRQHandler(void){
    exti_handle(3);
}

void EXINT4_IRQHandler(void){
    exti_handle(4);
}

void EXINT9_5_IRQHandler(void){
    exti_handle(5);
    exti_handle(6);
    exti_handle(7);
    exti_handle(8);
    exti_handle(9);
}

void EXINT15_10_IRQHandler(void){
    exti_handle(10);
    exti_handle(11);
    exti_handle(12);
    exti_handle(13);
    exti_handle(14);
    exti_handle(15);
}

}


