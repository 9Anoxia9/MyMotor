#ifndef __MOTOR_HPP__
#define __MOTOR_HPP__

#include "arm_math.h"
#include "at32f45x.h"
#include "foc.hpp"
#include <optional>

class Axis;


class Motor {
public:
        struct Config_t{
            uint8_t pole_pair = 7;      //7对极
            uint8_t R = 0.12f;          //0.12R
            uint8_t L = 0.00005;        //50uH
            float bandwidth = 6300;     //rad/s 1002Hz
            
            float dc_cail_tau = 2*PI;   //对应1Hz
        };

        Motor(tmr_type* tmr):_tmr(tmr){}

        void init();

        void update_current_control_law_pi_gain();

        void measure_current(std::optional<I_phUVW> current);

        void dc_measure_noise(std::optional<I_phUVW> current);

        void dc_cali_update(std::optional<I_phUVW> current);

        void arm(PhaseCtrlLaw* ctrl_law);

        void disarm();

        void update();

        void pwm_update(uint32_t output_timestamp);

        Config_t  _config;

        uint32_t _dc_cail_cnt  = 0;
        I_phUVW _dc_cail_sum = {0.0f};
        std::optional<I_phUVW> _dc_cali_avrg;
        std::optional<I_phUVW> _dc_cali_sigma_noise;  

        //电机状态
        bool is_armed = false;

        //外部输入
        float *_phase_src = nullptr;         //电角度rad
        float *_phase_vel_src = nullptr;     //电角速度rad/s
        float *_vd_set_src = nullptr;
        float *_vq_set_src = nullptr;
        float *_id_set_src = nullptr;            //由外部输入设置    
        float *_iq_set_src = nullptr;            //由外部输入设置

        I_phUVW _i_pha;

        //输出
        float _vd_set = 0.0f;            
        float _vq_set = 0.0f;
        float _id_set = 0.0f;
        float _iq_set = 0.0f;

        FOC _current_control_law;                //电流控制律
        VFControl _vf_open_control_law;          //VF开环控制律

        PhaseCtrlLaw* _ctrl_law;             //控制律

        Axis* axis = nullptr;

private:
        tmr_type* _tmr;
};

static inline float* get_motor_iu_ptr(Motor* mt){
        float* ptr_r = &(mt->_i_pha.iu) ;
        return ptr_r;
}

static inline float* get_motor_iv_ptr(Motor* mt){
        float* ptr_r = &(mt->_i_pha.iv);
        return ptr_r;
}

static inline float* get_motor_iw_ptr(Motor* mt){
        float* ptr_r = &(mt->_i_pha.iw);
        return ptr_r;
}

#endif
