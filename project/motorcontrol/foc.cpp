#include "foc.hpp"
#include "utils.hpp"
#include "board.h"

void FOC::init(){
    _integral_d = 0;   //总积分
    _integral_q = 0;   //总积分

    _i_timestamp = 0;
    _ctrl_timestamp = 0;
}

void FOC::reset(){
    _vd_set_src = nullptr;
    _vq_set_src = nullptr;
    _id_set_src = nullptr;
    _iq_set_src = nullptr;
    _phase_src = nullptr;
    _phase_vel_src = nullptr;
}

void FOC::update(uint32_t ctrl_timestamp){
    _ctrl_timestamp = ctrl_timestamp;
}

void FOC::measure_alpha_beta(uint32_t i_timestamp,I_phUVW i_ph){
    _i_timestamp = i_timestamp;
    _i_alpha = i_ph.iu;
    _i_beta  = ONE_BY_SQRT3*(i_ph.iv) - ONE_BY_SQRT3*(i_ph.iw);
}

void FOC::get_out(uint32_t output_timestamp,I_alpha_beta* mod_alpha_beta){
    
    float _phase_vel = *_phase_vel_src;
    float _phase = *_phase_src;
    float _vd_set = *_vd_set_src;
    float _vq_set = *_vq_set_src;
    float _id_set = *_id_set_src;
    float _iq_set = *_iq_set_src;
    float mod_d,mod_q;
    
    float phase = _phase + _phase_vel*((float)(int32_t)(_i_timestamp-_ctrl_timestamp)/(float)TIM1_8_SRC_FRQ);
    float id = _i_alpha*arm_cos_f32(phase) + _i_beta*arm_sin_f32(phase);
    float iq = -_i_alpha*arm_sin_f32(phase) + _i_beta*arm_cos_f32(phase);


    float max_ph_vol_inv = 1/(MAX_VOLTAGE*TWO_BY_3);   //最大相电压的倒数.
    float Id_err = _id_set - id;
    float Iq_err = _iq_set - iq;
    float mod_vector;

    mod_d = (_vd_set + Id_err * _kp + _integral_d)*max_ph_vol_inv;
    mod_q = (_vq_set + Iq_err * _kp + _integral_q)*max_ph_vol_inv;

    arm_sqrt_f32(mod_d*mod_d+mod_q*mod_q,&mod_vector);

    if(mod_vector > SQRT3_BY_2*0.8f){    //限制在线性区
        float scale = SQRT3_BY_2/mod_vector;
        mod_d *= scale;
        mod_q *= scale;
        _integral_d*=0.99f;
        _integral_q*=0.99f;
    }else{
        _integral_d += _ki * Id_err * CTRL_DT;
        _integral_q += _ki * Iq_err * CTRL_DT; 
    }

    _phase += _phase_vel*((float)(int32_t)(output_timestamp - _ctrl_timestamp)/(float)TIM1_8_SRC_FRQ);
    phase = wrap_pi(_phase);
    mod_alpha_beta->mod_a = mod_d*arm_cos_f32(_phase) - mod_q*arm_sin_f32(_phase);
    mod_alpha_beta->mod_b = mod_d*arm_sin_f32(_phase) + mod_q*arm_cos_f32(_phase);
}


void VFControl::init(){
    _i_timestamp = 0;
    _ctrl_timestamp = 0;
}

void VFControl::reset(){
   _vq_set_src = nullptr;
   _vd_set_src = nullptr;
   _phase_src = nullptr;
   _phase_vel_src = nullptr;
}

void VFControl::update(uint32_t ctrl_timestamp){
    _ctrl_timestamp = ctrl_timestamp;
}

void VFControl::get_out(uint32_t output_timestamp,I_alpha_beta* alpha_beta){
    float _phase = *_phase_src;
    float _phase_vel = *_phase_vel_src;
    float _vd_set = *_vd_set_src;
    float _vq_set = *_vq_set_src;
    float mod_d,mod_q,mod_vector;

    float max_ph_vol_inv = 1/(MAX_VOLTAGE*TWO_BY_3);   //最大相电压的倒数.
    mod_d = _vd_set*max_ph_vol_inv;
    mod_q = _vq_set*max_ph_vol_inv;

    arm_sqrt_f32(mod_d*mod_d+mod_q*mod_q,&mod_vector);

    if(mod_vector > SQRT3_BY_2){
        float scale = SQRT3_BY_2/mod_vector;
        mod_d *= scale;
        mod_q *= scale;
    }

    _phase = _phase + _phase_vel*((int32_t)output_timestamp-(int32_t)_ctrl_timestamp)/TIM1_8_SRC_FRQ;
    _phase = wrap_pi(_phase);

    alpha_beta->mod_a = mod_d*arm_cos_f32(_phase) - mod_q*arm_sin_f32(_phase);
    alpha_beta->mod_b = mod_d*arm_sin_f32(_phase) + mod_q*arm_cos_f32(_phase);
}

