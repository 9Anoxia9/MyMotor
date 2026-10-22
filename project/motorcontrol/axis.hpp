#ifndef __AXIS_HPP__
#define __AXIS_HPP__


#include "openloop.hpp"
#include "motor.hpp"
#include "encoder.hpp"
#include "control.hpp"

class Axis{
public:
        enum AxisStatus{
            AXIS_STATUS_UNDEFINE = 0,
            AXIS_STATUS_IDLE = 1,
            AXIS_STATUS_DIR_CALIBRATION = 2,
            AXIS_STATUS_OFFSET_CALIBRATION = 3,
            AXIS_STATUS_OPEN_LOOP = 4,
            AXIS_STATUS_CLOSE_LOOP = 5,
        };        


        Axis(uint8_t axis_idx,Encoder& encoder,Motor& mt,Control& control):
                _axis_idx(axis_idx),_encoder(encoder),_mt(mt),_control(control){
            _encoder.axis = this;
            _mt.axis = this;
            _VF_openloop_controller._axis = this;
            _control._axis = this;
        }

        void init();

        void run_open_loop();

        void run_close_loop();

        void run_idle();

        void run_direction_calibration();

        void run_offset_calibration();

        void thread_wakeup();

        static void task(void* ctx);

        uint8_t _axis_idx;

        OpenloopVF  _VF_openloop_controller;
        Encoder&    _encoder;
        Motor&      _mt;
        Control&    _control;

        AxisStatus _status = AXIS_STATUS_UNDEFINE;
        AxisStatus _status_chain[1] = {AXIS_STATUS_UNDEFINE};

        TaskHandle_t _task;
        SemaphoreHandle_t _sem;
};

static inline void set_openloop_target_vel(Axis* axis,float ph_vel){
        axis->_VF_openloop_controller._target_vel = ph_vel;
}

static inline Axis::AxisStatus* get_axis_status(Axis* axis){
        return &(axis->_status);
}


static inline void set_axis_status(Axis* axis,uint8_t status){
        axis->_status = (Axis::AxisStatus)status;
}

#endif
