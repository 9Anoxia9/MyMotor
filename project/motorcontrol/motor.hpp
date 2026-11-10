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
            uint8_t pole_pair = 7;                 //7对极
            float R = 0.21413984894752502f;        //0.12R 0.21413984894752502f
            float L = 0.000054197236750042066f;    //50uH  0.000054197236750042066f

            float tuque_const = 60.0f/2.0f/PI/360.0f*SQRT3_BY_2;

            float max_torque = 0.115f;   //Nm

            float J = 0.000014337699f;

            float bandwidth = 1000;               //rad/s 1002Hz 6300

            float dc_cail_tau = 1.0f/(2.0f*PI);   //对应1Hz

            float max_current = 4.0f;   //A

            float torque_dir = 1.0f;
        };

        Motor(tmr_type* tmr):_tmr(tmr){}

        void init();

        void update_current_control_law_pi_gain();

        void measure_current(uint32_t timestamp,std::optional<I_phUVW> current);

        void dc_measure_noise(std::optional<I_phUVW> current);

        void dc_cali_update(std::optional<I_phUVW> current);

        void arm(PhaseCtrlLaw* ctrl_law);

        void disarm();

        void update();

        void pwm_update(uint32_t output_timestamp);

        PhaseCtrlLaw* get_ctrl_law(){
            if(_ctrl_law == &_vf_open_control_law){
                return &_vf_open_control_law;
            }else{
                return &_current_control_law;
            }
        }

        Config_t  _config;

        //测量校准电流值
        I_phUVW _i_pha;
        I_phUVW _adc_val;
        std::optional<I_phUVW> _measure_i;
        
        //直流偏置基准值
        I_phUVW _dc_cail_i = {0.0f};
        float _dc_cail_elapsed_time = 0.0f;

        //测量量化噪声
        uint32_t _dc_cail_cnt  = 0;
        I_phUVW _dc_cail_sum = {0.0f};
        std::optional<I_phUVW> _dc_cali_avrg = std::nullopt;
        std::optional<I_phUVW> _dc_cali_sigma_noise = std::nullopt;  

        //电机状态
        bool _is_armed = false;

        //外部输入
        float *_phase_vel_src = nullptr;     //电角速度rad/s
        float *_torque_src = nullptr;    //扭矩
        float *_id_src = nullptr;
        float *_iq_src = nullptr;

        //输出
        float _id_set = 0.0f;
        float _iq_set = 0.0f;
        float _vd_set = 0.0f;            
        float _vq_set = 0.0f;

        FOC _current_control_law;                //电流控制律
        VFControl _vf_open_control_law;          //VF开环控制律

        PhaseCtrlLaw* _ctrl_law;             //控制律

        Axis* axis = nullptr;

private:
        tmr_type* _tmr;
};

static inline float* get_motor_u_adc_ptr(Motor* mt){
        float* ptr_r = &(mt->_adc_val.iu) ;
        return ptr_r;
}

static inline float* get_motor_v_adc_ptr(Motor* mt){
        float* ptr_r = &(mt->_adc_val.iv);
        return ptr_r;
}

static inline float* get_motor_w_adc_ptr(Motor* mt){
        float* ptr_r = &(mt->_adc_val.iw);
        return ptr_r;
}

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

static inline float* get_motor_id_set(Motor* mt){
        float* ptr_r = &(mt->_id_set);
        return ptr_r;
}

static inline float* get_motor_iq_set(Motor* mt){
        float* ptr_r = &(mt->_iq_set);
        return ptr_r;
}

static inline float* get_motor_id_feedback(Motor* mt){
        float* ptr_r = &(mt->get_ctrl_law()->_id_feedback);
        return ptr_r;
}

static inline float* get_motor_iq_feedback(Motor* mt){
        float* ptr_r = &(mt->get_ctrl_law()->_iq_feedback);
        return ptr_r;
}

static inline float* get_motor_ia_feedback(Motor* mt){
        float* ptr_r = &(mt->get_ctrl_law()->_i_alpha);
        return ptr_r;
}

static inline float* get_motor_ib_feedback(Motor* mt){
        float* ptr_r = &(mt->get_ctrl_law()->_i_beta);
        return ptr_r;
}

static inline float* get_motor_integral_d(Motor* mt){
        float* ptr_r = &(mt->_current_control_law._integral_d);
        return ptr_r;
}

static inline float* get_motor_integral_q(Motor* mt){
        float* ptr_r = &(mt->_current_control_law._integral_q);
        return ptr_r;
}

static inline float* get_motor_mod_q(Motor* mt){
        float* ptr_r = &(mt->_current_control_law._mod_q);
        return ptr_r;
}

static inline float* get_motor_mod_d(Motor* mt){
        float* ptr_r = &(mt->_current_control_law._mod_d);
        return ptr_r;
}

#endif
