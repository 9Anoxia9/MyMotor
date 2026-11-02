#include "board.h"
#include "utils.hpp"
#include <algorithm>


void Control::init(){
    _pos_kp = _config.pos_bandwidth;

    _vel_kp = 2 * _config.vel_bandwidth * _axis->_mt._config.J;
    _vel_ki = SQ(_config.vel_bandwidth) * _axis->_mt._config.J;
}

void Control::update(){
    float pos_err = 0.0f;
    float vel_err = 0.0f;

    _vel_cmd = _vel_input;
    if(_config.mode <= POS_CTRL){
        pos_err = _pos_input - (*_pos_src) * 2 * PI;
        _vel_cmd = _pos_kp * pos_err/5.0f;
    }
    
    _torque_cmd = _torque_input;
    if(_config.mode <= VEL_CTRL){

        _vel_cmd = std::clamp(_vel_cmd,-_config.max_vel,_config.max_vel);


        //if(_config.mode == VEL_CTRL){
            float max_step = 100.0f * 2.0f * PI * CTRL_DT;
            float step = std::clamp(_vel_cmd - _vel_ramp_cmd, -max_step, max_step);

            _vel_ramp_cmd += step;
            _torque_cmd = step/CTRL_DT * _axis->_mt._config.J;
            vel_err = _vel_ramp_cmd - (*_vel_src) * 2.0f * PI;
        /*}else{
            vel_err = _vel_cmd - (*_vel_src) * 2 * PI;
        }*/

        _torque_cmd += _vel_kp * vel_err + _vel_integral;

        if(std::abs(_torque_cmd) >= _axis->_mt._config.max_torque){
            _vel_integral *= 0.99f;
        }else{
            _vel_integral += vel_err * _vel_ki * CTRL_DT;
        }
    }

    if(_config.mode <= TORQUE_CTRL){
        float max_torque = _axis->_mt._config.max_torque;

        _torque_cmd = std::clamp(_torque_cmd,-max_torque,max_torque);
    }
}

void Control::set_pos(float pos){
    _config.mode = POS_CTRL;
    _pos_input = pos*2*PI;
    _vel_input = 0;
    _torque_input = 0;
}

void Control::set_vel(float vel){
    _config.mode = VEL_CTRL;
    _vel_input = vel*2*PI;
    _pos_input = 0;
    _torque_input = 0;
}

void Control::set_torque(float torque){
    _config.mode = TORQUE_CTRL;
    _torque_input = torque;
    _vel_input = 0;
    _pos_input = 0;
}