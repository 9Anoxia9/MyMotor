#include "motor.hpp"
#include "foc.hpp"
#include "board.h"
#include "utils.hpp"


void Motor::init(){
    update_current_control_law_pi_gain();
}

void Motor::update_current_control_law_pi_gain(){
    _current_control_law._kp = _config.bandwidth*_config.L;
    _current_control_law._ki = _config.bandwidth*_config.R;
}

void Motor::measure_current(std::optional<I_phUVW> current){
    
}

void Motor::dc_measure_noise(std::optional<I_phUVW> current){
    if(current.has_value())

    if(_dc_cali_avrg.has_value()){
        if(_dc_cail_cnt < 10000){
            _dc_cail_sum.iu += current->iu;
            _dc_cail_sum.iv += current->iu;
            _dc_cail_sum.iw += current->iu;
            _dc_cail_cnt++;
        }else{
            _dc_cali_avrg->iu =  _dc_cail_sum.iu/10000;
            _dc_cali_avrg->iu =  _dc_cail_sum.iv/10000;
            _dc_cali_avrg->iu =  _dc_cail_sum.iw/10000;
        }
    }
}

void Motor::dc_cali_update(std::optional<I_phUVW> current){

}



void Motor::arm(PhaseCtrlLaw* ctrl_law){

    CRITICAL_SECTION{
        if(ctrl_law != nullptr){
            _ctrl_law = ctrl_law;
            is_armed = true;

            ctrl_law->reset();
        }
    }
}

void Motor::disarm(){
    CRITICAL_SECTION{
        _ctrl_law = nullptr;
        is_armed = false;
    }
}

void Motor::update(){
    
}

void Motor::pwm_update(uint32_t output_timestamp){

    I_alpha_beta mod_alpha_beta = {0,0};

    if(_ctrl_law != nullptr){
        _ctrl_law->get_out(output_timestamp,&mod_alpha_beta);
    }

    //然后调用SVPWM,输入mod_a、mod_b
    auto [tU,tV,tW,success] = SVPWM(mod_alpha_beta.mod_a,mod_alpha_beta.mod_b);
    
    if(!success){
        return;
    }else{
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_1, (uint16_t)(tU*(float)TIM1_8_PERIOD) );
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_2, (uint16_t)(tV*(float)TIM1_8_PERIOD) );
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_3, (uint16_t)(tW*(float)TIM1_8_PERIOD) );
    }
}