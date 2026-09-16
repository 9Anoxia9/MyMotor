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

    /**
     * 这里用1/√3而不是2/3是因为用2/3会过早进入过调制区
     * 而电机常数KV是在线性情况下测出的
     *  */
    _vq_set = _kvf*_cur_vel*ONE_BY_SQRT3;
    if(_vq_set >= 0 && _target_vel > 0){
        _vq_set += _low_vel_compensate_voltage;
    }else if(_vq_set <= 0 && _target_vel < 0){
        _vq_set -= _low_vel_compensate_voltage;
    }
    
    _cur_vel += ki*get_err()*CTRL_DT;
}