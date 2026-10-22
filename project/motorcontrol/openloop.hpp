#ifndef __OPENLOOP_HPP__
#define __OPENLOOP_HPP__

#include "utils.hpp"
#include "foc.hpp"


class Axis;
class VFControl;

class OpenloopVF{
public:
        void update();
        float get_err();


        float _kvf = 24.0f*60.0f/8640.0f*ONE_BY_SQRT3;   //kvf = max_voltage*60s/max_vel(turn/min)
        float _low_vel_compensate_voltage = 0.5f; //V
        float ki = 10.0f; 
        
        float _cur_vel = 0.0f;          //turn/s,当前转速

        //输入
        float _target_vel = 0.0f;       //turn/s,目标转速,外部输入设置

        //输出
        float _vd_set = 0.0f;
        float _vq_set = 0.0f;           //V
        float _phase_vel = 0.0f;        //rad/s
        float _phase = 0.0f;            //rad

        float _total_turn = 0.0f;

        Axis* _axis = nullptr;
        
};

#endif
