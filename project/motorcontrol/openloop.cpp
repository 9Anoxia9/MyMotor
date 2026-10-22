#include "board.h"

float OpenloopVF::get_err(){

    float err = _target_vel - _cur_vel;

    if(std::abs(err) >= 2.0f){
        if(err > 0){
            return 2.0f;
        }else{
            return -2.0f;
        }
    }else{
        return err;
    }
}

void OpenloopVF::update(){

    _phase_vel = _cur_vel*2*PI*_axis->_mt._config.pole_pair;
    _phase += _phase_vel*CTRL_DT;
    _phase = wrap_pi(_phase);

    _total_turn += _cur_vel*CTRL_DT;

    _vq_set = _kvf*_cur_vel;
    if(_vq_set >= 0 && _target_vel > 0){
        _vq_set += _low_vel_compensate_voltage;
    }else if(_vq_set <= 0 && _target_vel < 0){
        _vq_set -= _low_vel_compensate_voltage;
    }
    
    _cur_vel += ki*get_err()*CTRL_DT;
}