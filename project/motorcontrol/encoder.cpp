#include "encoder.hpp"
#include "utils.hpp"
#include "my_debug.h"
#include "board.h"


void Encoder::abs_spi_cs_pin_init(){
    switch(_config.abs_ncs_gpio_pin){
        case 1:
                _abs_ncs_gpio.set_gpio(GPIOA,GPIO_PINS_1);
                break;
        case 2:
                _abs_ncs_gpio.set_gpio(GPIOA,GPIO_PINS_2);
                break;
        case 3:
                _abs_ncs_gpio.set_gpio(GPIOA,GPIO_PINS_3);
                break;
        case 4:
                _abs_ncs_gpio.set_gpio(GPIOA,GPIO_PINS_4);
                break;
        case 5:
                _abs_ncs_gpio.set_gpio(GPIOA,GPIO_PINS_5);
                break;
    }

    _abs_ncs_gpio.set_output(GPIO_OUTPUT_PUSH_PULL,GPIO_PULL_NONE);
    _abs_ncs_gpio.write(true);
}

void Encoder::init(){
    abs_spi_cs_pin_init();

    _mode = _config.mode;
    _pll_kp = _config.pll_kp;
    _pll_ki = _config.pll_ki;
}

void Encoder::abs_spi_sample(){

    if(_mode != MODE_ABS_AS5047P){
        return;
    }

    _spitask = {
        _abs_ncs_gpio,
        _abs_spi_tx_data,
        &_abs_spi_rx_data,
        [](void* ctx,bool success){((Encoder*)ctx)->abs_spi_cb(success);},
        this,
        false,
        nullptr
    };

    if(_spi.check_task_is_vaild(&_spitask)){
        _spi.start(&_spitask);
    }else{
        LOG("spitask invaild\n");
    }
}

void Encoder::abs_spi_cb(bool success){
    
    if(success == false){
        return;
    }
    
    if(_mode == MODE_ABS_AS5047P){
        if(parity15(_abs_spi_rx_data) == (_abs_spi_rx_data>>15)){
            _sample_cnt_in_cpr = _abs_spi_rx_data & 0x3FFF;
            _abs_spi_update = true;
        }
    }
}

void Encoder::update(){
    int32_t delta_pos = 0;
    int32_t sample_cnt_in_cpr = _sample_cnt_in_cpr;

    while(spi_i2s_flag_get(*_spi, SPI_I2S_BF_FLAG) != RESET);
    gpios[CLK_IO].write(0);

    if(_mode == MODE_ABS_AS5047P){
        if(_abs_spi_update == false){
            return;
        }else{

        }

        float mech_rad = (float)sample_cnt_in_cpr * 2.0f * PI / (float)_config.cpr;
        int32_t comp = (int32_t)lroundf(45.0f * arm_sin_f32(4.0f * mech_rad + _config._err4_phase));
        sample_cnt_in_cpr -= comp;
        if(sample_cnt_in_cpr < 0){
            sample_cnt_in_cpr += (int32_t)_config.cpr;
        }else if(sample_cnt_in_cpr >= (int32_t)_config.cpr){
            sample_cnt_in_cpr -= (int32_t)_config.cpr;
        }

        delta_pos = sample_cnt_in_cpr - _cnt_in_cpr;
        /**
         * 将delta_pos映射到[-cpr/2,cpr/2]。在分辨率16384的情况下,125us不可能
         * 的位置变化不可能超过cpr/2,若突然出现超过cpr/2必然是某一方向旋转出现回绕
         */
        delta_pos = wrap_half(delta_pos,_config.cpr);
    }
    _abs_spi_update = false;

    _total_pos_cnt += delta_pos;                //记录实际多圈位置计数
    _cnt_in_cpr = sample_cnt_in_cpr;            //记录单圈实际位置计数

    _cpr_pos_est_cnt += _vel_est_cnt * CTRL_DT;
    _cpr_pos_est_cnt = wrap_pos(_cpr_pos_est_cnt,_config.cpr);

    float delta_cpr_pos = _cnt_in_cpr - _cpr_pos_est_cnt;          //单圈误差
    //单圈误差区分方向
    delta_cpr_pos = wrap_half(delta_cpr_pos,_config.cpr);

    _cpr_pos_est_cnt += _pll_kp * delta_cpr_pos * CTRL_DT;
    _cpr_pos_est_cnt = wrap_pos(_cpr_pos_est_cnt,_config.cpr);

    _vel_est_cnt += _pll_ki * delta_cpr_pos * CTRL_DT;   //单圈误差小,多圈存在误差累积

    _pos_est = (float)_total_pos_cnt/_config.cpr;       //turn
    _vel_est = _vel_est_cnt/_config.cpr;                //turn/s

    int32_t correct_cnt_in_cpr = (int32_t)_cnt_in_cpr - (int32_t)_config.phase_offset_cpr;
    float elec_rad_per_enc = 2.0f*PI*7.0f/_config.cpr;
    float ph = correct_cnt_in_cpr*elec_rad_per_enc;

    _phase = wrap_pi(ph)*_config.direction;
    _phase_vel = _vel_est*2.0f*PI*7.0f*_config.direction;
}
