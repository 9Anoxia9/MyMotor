#ifndef __CONTROL_HPP__
#define __CONTROL_HPP__

class Axis;

enum control_mode{
    POS_CTRL = 0,
    VEL_CTRL,
    TORQUE_CTRL
};

class Control{
public:

        struct Config_t{
            enum control_mode mode = POS_CTRL;

            float pos_bandwidth = 2.0f*2.0f*PI;     //2Hz
            float vel_bandwidth = 20.0f*2.0f*PI;    //10Hz

            float max_vel = 288*PI;                   //rad/s
        };

        Control(){}

        void init();

        void update();

        void set_pos(float pos);
        void set_vel(float vel);
        void set_torque(float torque);

        Config_t _config;

        //上位机输入
        float _pos_input = 0.0f;        //rad
        float _vel_input = 0.0f;        //rad/s
        float _torque_input = 0.0f;     //N*m

        //位置环参数
        float *_pos_src = nullptr;      //rad
        float _pos_kp = 0.0f;           //1/s
        float _vel_cmd = 0.0f;          //rad/s
        float _vel_ramp_cmd = 0.0f;     //rad/s

        //速度环参数
        float *_vel_src = nullptr;      //rad/s
        float _vel_kp = 0.0f;           //N*m/(rad/s)
        float _vel_ki = 0.0f;           //N*m/rad
        float _vel_integral = 0.0f;     

        //扭矩环参数
        float _torque_cmd = 0.0f;

        Axis* _axis = nullptr;
};

static inline void set_control_pos(Control& ct,float pos){
    ct.set_pos(pos);
}

static inline void set_control_vel(Control& ct,float vel){
    ct.set_vel(vel);
}

static inline void set_control_torque(Control& ct,float torque){
    ct.set_torque(torque);
}

#endif
