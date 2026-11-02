#include "motor.hpp"
#include "foc.hpp"
#include "board.h"
#include "utils.hpp"
#include <algorithm>


void Motor::init(){
    update_current_control_law_pi_gain();
}

void Motor::update_current_control_law_pi_gain(){
    _current_control_law._kp = _config.bandwidth*_config.L;
    _current_control_law._ki = _config.bandwidth*_config.R;
}

void Motor::measure_current(uint32_t timestamp,std::optional<I_phUVW> current){
    bool dc_cail_enable = (_dc_cail_elapsed_time >= _config.dc_cail_tau*1.5f*2*PI);

    if(dc_cail_enable && current.has_value()){
        _i_pha = {
            current->iu - _dc_cail_i.iu,
            current->iv - _dc_cail_i.iv,
            current->iw - _dc_cail_i.iw,
        };
        _measure_i = _i_pha;
    }else{
        _measure_i = std::nullopt;
    }

    if(_measure_i.has_value()){
        float pha_i = SQ(_measure_i->iu)+SQ(_measure_i->iv)+SQ(_measure_i->iw);

        if(SQ(_config.max_current) < TWO_BY_3*pha_i){   //超过了最大电流
            //disarm();
        }
    }else{
        if(_is_armed == true){
            disarm();
        }
    }

    if(_ctrl_law != nullptr){
        _ctrl_law->measure_alpha_beta(timestamp,{_measure_i->iu,_measure_i->iv,_measure_i->iw});
    }

}

void Motor::dc_measure_noise(std::optional<I_phUVW> current){
    if(current.has_value() == false){
        _dc_cail_cnt = 0;
        _dc_cail_sum = {0.0f};
        _dc_cali_avrg = std::nullopt;
        _dc_cali_sigma_noise = std::nullopt;
        return;
    }

    if(_dc_cali_avrg.has_value() == false){
        if(_dc_cail_cnt < 10000){
            _dc_cail_sum.iu += current->iu;
            _dc_cail_sum.iv += current->iv;
            _dc_cail_sum.iw += current->iw;
            _dc_cail_cnt++;
        }else{
            _dc_cali_avrg = I_phUVW{
                                _dc_cail_sum.iu/10000,
                                _dc_cail_sum.iv/10000,
                                _dc_cail_sum.iw/10000,
                            };
            _dc_cail_sum = {0.0f};
            _dc_cail_cnt = 0;
        }
    }

    if(_dc_cali_avrg.has_value() == true && _dc_cali_sigma_noise.has_value() == false){
        if(_dc_cail_cnt < 10000){
            float iu_err = current->iu - _dc_cali_avrg->iu;
            float iv_err = current->iv - _dc_cali_avrg->iv;
            float iw_err = current->iw - _dc_cali_avrg->iw;

            _dc_cail_sum.iu += iu_err*iu_err;
            _dc_cail_sum.iv += iv_err*iv_err;
            _dc_cail_sum.iw += iw_err*iw_err;
            _dc_cail_cnt++;
        }else{
            _dc_cali_sigma_noise = I_phUVW{
                                        _dc_cail_sum.iu/9999,
                                        _dc_cail_sum.iv/9999,
                                        _dc_cail_sum.iw/9999
                                    }; 
            _dc_cail_sum = {0.0f};
            _dc_cail_cnt = 0;
        }
    }
}

void Motor::dc_cali_update(std::optional<I_phUVW> current){
    float K = CTRL_DT/_config.dc_cail_tau;

    if(current.has_value()){
        _dc_cail_i.iu += K*(current->iu - _dc_cail_i.iu);
        _dc_cail_i.iv += K*(current->iv - _dc_cail_i.iv); 
        _dc_cail_i.iw += K*(current->iw - _dc_cail_i.iw);
        _dc_cail_elapsed_time += CTRL_DT;
    }else{
        _dc_cail_i.iu = 0;
        _dc_cail_i.iv = 0;
        _dc_cail_i.iw = 0;
        _dc_cail_elapsed_time = 0;
    }
}

void Motor::arm(PhaseCtrlLaw* ctrl_law){

    CRITICAL_SECTION{
        if(ctrl_law != nullptr){
            _ctrl_law = ctrl_law;
            _is_armed = true;

            ctrl_law->init();
        }
    }
}

void Motor::disarm(){
    CRITICAL_SECTION{
        _ctrl_law = nullptr;
        _is_armed = false;
    }
}

void Motor::update(){
    float phase_vel = *_phase_vel_src;
    float torque;

    if(_torque_src != nullptr){     //扭矩控制
        torque = *_torque_src;
        torque *= _config.torque_dir;
        _iq_set = torque/_config.tuque_const;
        _id_set = 0.0f;
    }else if(_id_src != nullptr){   //IF
        _id_set = *_id_src;
        _iq_set = 0.0f;
    }
    
    if(_ctrl_law == &_current_control_law){
        _vd_set = _id_set * _config.R - phase_vel*_config.L * _iq_set;
        _vq_set = _iq_set * _config.R + phase_vel*_config.L * _id_set;
        _vq_set += TWO_BY_3 * phase_vel * _config.tuque_const / _config.pole_pair;
    }
}

void Motor::pwm_update(uint32_t output_timestamp){
    float deadtime_comp = 0.0f;
    I_alpha_beta mod_alpha_beta = {0,0};

    if(_ctrl_law != nullptr){
        _ctrl_law->get_out(output_timestamp,&mod_alpha_beta);
        deadtime_comp = 0.0125f;
    }

    //然后调用SVPWM,输入mod_a、mod_b
    auto [tU,tV,tW,success] = SVPWM(mod_alpha_beta.mod_a,mod_alpha_beta.mod_b);

   /*if(!(tU == 0.5f && tV == 0.5f && tW == 0.5f)){
        static uint8_t comp_u_on = false,comp_v_on = false,comp_w_on = false;
        

        if(!comp_u_on && _measure_i->iu > 0.05f)  comp_u_on = true;   // 开启阈值：0.05A
        if( comp_u_on && _measure_i->iu < 0.02f)  comp_u_on = false;

        if(!comp_v_on && _measure_i->iv > 0.05f)  comp_v_on = true;   // 开启阈值：0.05A
        if( comp_v_on && _measure_i->iv < 0.02f)  comp_v_on = false;

        if(!comp_w_on && _measure_i->iw > 0.05f)  comp_w_on = true;   // 开启阈值：0.05A
        if( comp_w_on && _measure_i->iw < 0.02f)  comp_w_on = false;

        if(comp_u_on){
            float tu_comp = deadtime_comp * std::clamp(_measure_i->iu / 0.05f, -1.0f, 1.0f);
            tU -= tu_comp;
        }
        if(comp_v_on){
            float tv_comp = deadtime_comp * std::clamp(_measure_i->iv / 0.05f, -1.0f, 1.0f);
            tV -= tv_comp;
        }
        if(comp_w_on){
            float tw_comp = deadtime_comp * std::clamp(_measure_i->iw / 0.05f, -1.0f, 1.0f);;
            tW -= tw_comp;
        }
    }*/
    
    
    if(!success){
        return;
    }else{
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_1, (uint16_t)(tU*(float)TIM1_8_PERIOD) );
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_2, (uint16_t)(tV*(float)TIM1_8_PERIOD) );
        tmr_channel_value_set(_tmr, TMR_SELECT_CHANNEL_3, (uint16_t)(tW*(float)TIM1_8_PERIOD) );
    }
}