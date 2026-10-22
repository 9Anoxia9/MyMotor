#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include "at32_gpio.hpp"
#include "at32_spi.hpp"
#include "utils.hpp"

class Axis;

typedef enum Eocode_Mode{
    MODE_INCREMENT,
    MODE_HALL,
    MODE_ABS_AS5047P
}e_mode;

#if __cplusplus

class Encoder {
public:
    struct config_t{
        e_mode mode = MODE_ABS_AS5047P;
        uint32_t cpr = 16384;

        uint32_t phase_offset_cpr = 0;

        uint16_t abs_ncs_gpio_pin = 1;

        float pll_bandwidth = 1000.0f;     //rad
        float pll_kp = 2*pll_bandwidth;
        float pll_ki = pll_bandwidth*pll_bandwidth;

        float _err4_phase = 0.0f;

        float direction = 1.0f;
    };

    Encoder(Spi& spi):_spi(spi){}
    
    struct config_t _config;

    e_mode _mode;

    uint16_t _sample_cnt_in_cpr;        //实时单圈位置值,它是采样值.0~16384
    uint16_t _cnt_in_cpr;               //记录上一时刻的单圈位置
    int32_t  _total_pos_cnt;            //记录实时位置

    float _cpr_pos_est_cnt = 0.0f;       //conut 估算单圈编码值
    float _vel_est_cnt = 0.0f;           //count/s 估算的速度值

    float _pos_est = 0.0f;               //turn
    float _vel_est = 0.0f;               //turn/s
    float _vel_simple = 0.0f;

    float _pll_kp;                      //
    float _pll_ki;

    float _phase;
    float _phase_vel;
    float _direction = 1.0f;

    void init();

    void update();

    void abs_spi_sample();
    void abs_spi_cb(bool success);
    void abs_spi_cs_pin_init();
    bool _abs_spi_update = false;
    Gpio _abs_ncs_gpio;
    Spi& _spi;
    uint16_t _abs_spi_tx_data = 0xFFFF;
    uint16_t _abs_spi_rx_data;
    struct Spi::SpiTask _spitask;

    Axis* axis = nullptr;
};

static inline e_mode* get_encoder_mode_ptr(Encoder* enc){
        e_mode* ptr_r = &(enc->_config.mode);
        return ptr_r;
}

static inline uint32_t* get_encoder_cpr_ptr(Encoder* enc){
        uint32_t* ptr_r = &(enc->_config.cpr);
        return ptr_r;
}

static inline uint16_t* get_encoder_abs_ncs_gpio_pin_ptr(Encoder* enc){
        uint16_t* ptr_r = &(enc->_config.abs_ncs_gpio_pin);
        return ptr_r;
}

static inline float* get_encoder_phase_ptr(Encoder* enc){
        float* ptr_r = &(enc->_phase);
        return ptr_r;
}

static inline float* get_encoder_phase_vel_ptr(Encoder* enc){
        float* ptr_r = &(enc->_phase_vel);
        return ptr_r;
}

static inline int32_t* get_encoder_total_pos_cnt_ptr(Encoder* enc){
        int32_t* ptr_r = &(enc->_total_pos_cnt);
        return ptr_r;
}

static inline float* get_encoder_pos_est_ptr(Encoder* enc){
        float* ptr_r = &(enc->_pos_est);
        return ptr_r;
}

static inline float* get_encoder_vel_est_ptr(Encoder* enc){
        float* ptr_r = &(enc->_vel_est);
        return ptr_r;
}

static inline uint8_t parity15(uint16_t x) {
    x &= 0x7FFF;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return x & 1;
}

#endif

#endif
