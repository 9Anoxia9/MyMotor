#ifndef __AT32_GPIO_HPP__
#define __AT32_GPIO_HPP__

#include "at32f45x.h"
#include "my_debug.h"

class Gpio {
public:
    static const Gpio none;

    Gpio(): port_(nullptr),pin_(0) {}
    Gpio(gpio_type* port,uint16_t pin): port_(port),pin_(pin) {}

    operator bool() {return port_ && pin_;}

    void port_clken();

    bool set_gpio( gpio_type*  port,uint16_t    pin){
        port_ = port; pin_ = pin;
        return true;
    }
    
    bool set_output(gpio_output_type type,gpio_pull_type pull);
    bool set_input(gpio_pull_type pull);
    bool set_analog();

    bool set_exint(exint_polarity_config_type polarity,
                      uint32_t preempt_priority = 5,
                      uint32_t sub_priority = 0,
                      void (*cb)(void*) = nullptr,void* ctx = nullptr);
    bool reset_exint();

    void write(bool state){
        if(state){
            gpio_bits_set(port_,pin_);
        }else{
            gpio_bits_reset(port_,pin_);
        }
    };

    bool read_input(){
        return port_ && (port_->idt & pin_);
    };

    bool read_output(){
        return port_ && (port_->odt & pin_);
    };

    uint32_t get_pin_number() const {
        ASSERT(pin_ != 0);
        return static_cast<uint32_t>(__builtin_ctz(pin_));
    }

    scfg_pins_source_type get_pin_source(){
        uint32_t pin_number = get_pin_number();
        ASSERT(pin_number <= 15);
        return static_cast<scfg_pins_source_type>(pin_number); 
    }

    IRQn_Type get_exint_iqr_number() const;

private:    /*只有成员函数可以修改private变量*/
    gpio_type*  port_;
    uint16_t    pin_;
};


#endif
