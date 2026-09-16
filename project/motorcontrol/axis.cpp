#include "axis.hpp"

void Axis::run_open_loop(){
    _mt._vf_open_control_law._vd_set_src = &_VF_openloop_controller._vd_set;
    _mt._vf_open_control_law._vq_set_src = &_VF_openloop_controller._vq_set;
    _mt._vf_open_control_law._phase_src = &_VF_openloop_controller._phase;
    _mt._vf_open_control_law._phase_vel_src = &_VF_openloop_controller._phase_vel;

    _mt.arm(&_mt._vf_open_control_law);
}