#include "board.h"
#include <algorithm>

#define PI_MULTI_2             6.283185307179f

float OpenloopVF::get_err(){

    float err = _vel_input - _cur_vel;

    if(std::abs(err) >= PI_MULTI_2){
        if(err > 0){
            return PI_MULTI_2;
        }else{
            return -PI_MULTI_2;
        }
    }else{
        return err;
    }
}

void OpenloopVF::update(){

    _phase_vel = _cur_vel * _axis->_mt._config.pole_pair;
    _phase += _phase_vel * CTRL_DT;
    _phase = wrap_pi(_phase);

    _total_turn += _cur_vel * CTRL_DT;

    _vq_set = _kvf*_cur_vel;
    if(_vq_set >= 0 && _vel_input > 0){
        _vq_set += _low_vel_compensate_voltage;
    }else if(_vq_set <= 0 && _vel_input < 0){
        _vq_set -= _low_vel_compensate_voltage;
    }
    
    _cur_vel += _vel_ramp*get_err()*CTRL_DT;
}

void OpenloopVF::set_vel_input(float vel){
    _vel_input = 2.0f * PI * vel;
}

void OpenloopVF::set_vel_ramp(float ramp){
    _vel_ramp = ramp;
}




void OpenloopIF::update(){
     
    _iq_set = std::clamp(_current_input,_iq_set - _current_ramp*CTRL_DT, \
                                                _iq_set + _current_ramp*CTRL_DT);
    _phase_vel = std::clamp(_vel_input,_phase_vel - _vel_ramp * CTRL_DT, \
                                                _phase_vel + _vel_ramp * CTRL_DT);
    _phase += _phase_vel * _axis->_mt._config.pole_pair * CTRL_DT;
    _phase = wrap_pi(_phase);
}

void OpenloopIF::set_current_input(float current){
    _current_input = current;
}

void OpenloopIF::set_vel_input(float vel){
    _vel_input = 2.0f * PI * vel;
}

void OpenloopIF::set_current_ramp(float ramp){
    _current_ramp = ramp;
}

void OpenloopIF::set_vel_ramp(float ramp){
    _vel_ramp = ramp;
}
