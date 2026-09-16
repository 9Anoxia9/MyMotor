#ifndef __AXIS_HPP__
#define __AXIS_HPP__


#include "openloop.hpp"
#include "motor.hpp"
#include "encoder.hpp"

class Axis{
public:
        
        Axis(uint8_t axis_idx,Encoder& encoder,Motor& mt):
                _axis_idx(axis_idx),_encoder(encoder),_mt(mt){
            _encoder.axis = this;
            _mt.axis = this;
            _VF_openloop_controller._axis = this;
        }

        void run_open_loop();

        uint8_t _axis_idx;

        OpenloopVF  _VF_openloop_controller;
        Encoder&    _encoder;
        Motor&      _mt;
};

static inline void set_openloop_target_vel(Axis* axis,float ph_vel){
        axis->_VF_openloop_controller._target_vel = ph_vel;
}

#endif
