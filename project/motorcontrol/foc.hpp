#ifndef __FOC_HPP__
#define __FOC_HPP__

#include "utils.hpp"

struct adc_phUVW {uint16_t u;uint16_t v;uint16_t w;}; 
struct I_phUVW {float iu;float iv;float iw;}; 
struct I_alpha_beta {float mod_a;float mod_b;}; 

class PhaseCtrlLaw{
public:

    virtual void init() = 0;

    virtual void reset() = 0;

    virtual void update(uint32_t ctrl_timestamp) = 0;

    virtual void measure_alpha_beta(uint32_t i_timestamp,I_phUVW i_ph) = 0;

    virtual void get_out(uint32_t output_timestamp,I_alpha_beta* alpha_beta) = 0;
};

class FOC : public PhaseCtrlLaw{
public:
    void init();

    void reset();

    void update(uint32_t ctrl_timestamp) final;

    void measure_alpha_beta(uint32_t i_timestamp,I_phUVW i_ph);

    void get_out(uint32_t output_timestamp,I_alpha_beta* mod_alpha_beta);


    //控制环参数
    float _kp;           //带宽*电感L
    float _ki;           //带宽*电阻
    float _integral_d;   //总积分
    float _integral_q;   //总积分
    float _id_err;       //记录误差
    float _iq_err;       //记录误差

    //外部输入
    float* _vd_set_src;     //目标电压，也是稳态时的电压
    float* _vq_set_src;     //目标电压，也是稳态时的电压
    float* _id_set_src;     //目标电流，也是稳态时的d轴电流    
    float* _iq_set_src;     //目标电流，也是稳态时的q轴电流
    float* _phase_vel_src;  //当前电角速度
    float* _phase_src;      //当前角度

    //采样转换,记录值,用于上位机读取
    float _i_alpha;
    float _i_beta;
    float _id_feedback = 0.0f;
    float _iq_feedback = 0.0f;
    float _mod_d = 0.0f;
    float _mod_q = 0.0f;

    //时刻
    uint32_t _i_timestamp;
    uint32_t _ctrl_timestamp;
};

class VFControl : public PhaseCtrlLaw{
public:
        void init();
        
        void reset();

        void update(uint32_t ctrl_timestamp) final;

        void measure_alpha_beta(uint32_t i_timestamp,I_phUVW i_ph);

        void get_out(uint32_t output_timestamp,I_alpha_beta* alpha_beta);

        float*  _vq_set_src;
        float*  _vd_set_src;
        float*  _phase_src;
        float*  _phase_vel_src;

        float _i_alpha;
        float _i_beta;
        float _id_feedback;
        float _iq_feedback;

        uint32_t _i_timestamp;
        uint32_t _ctrl_timestamp;
};




#endif