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
     
    if(_input_current == 0.0f && _input_vel == 0.0f){
        if(_cur_vel){
            _stop = true;
        }
    }
    
    if(_stop == false){
        _iq_set = std::clamp(_input_current,_iq_set - _current_ramp*CTRL_DT, \
                                                    _iq_set + _current_ramp*CTRL_DT);
    }
    _id_set = 0.0f;
    
    _cur_vel = std::clamp(_input_vel,_cur_vel - _vel_ramp * CTRL_DT, \
                                                _cur_vel + _vel_ramp * CTRL_DT);
    if(_stop){
        if(_cur_vel == 0.0f){
            _stop = false;
        }
    }

    _phase_vel =  _cur_vel * _axis->_mt._config.pole_pair;                                           
    _phase += _phase_vel * CTRL_DT;
    _phase = wrap_pi(_phase);
}

void OpenloopIF::set_current_input(float current){
    if(current == 0.0f){
        _input_current = 0.0f;
        _input_vel = 0.0f;
    }else{
        _input_current = current;
    }
}

void OpenloopIF::set_vel_input(float vel){
    if(vel == 0.0f){
        _input_vel = 0.0f;
        _input_current = 0.0f;
    }else{
        _input_vel = 2.0f * PI * vel;
        CRITICAL_SECTION{
            if(_stop){
                _stop = false;
            }
        }
    }
}

void OpenloopIF::set_current_ramp(float ramp){
    _current_ramp = ramp;
}

void OpenloopIF::set_vel_ramp(float ramp){
    _vel_ramp = ramp;
}
