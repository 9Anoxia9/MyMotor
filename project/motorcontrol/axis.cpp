#include "axis.hpp"
#include "my_debug.h"

void Axis::init(){
    _sem = xSemaphoreCreateBinary();
    xTaskCreate(task,"axis",LOG_STACK_SIZE,this,configMAX_PRIORITIES - 2,&_task);
}

void Axis::run_open_loop(){
    _mt._vf_open_control_law._vd_set_src = &_VF_openloop_controller._vd_set;
    _mt._vf_open_control_law._vq_set_src = &_VF_openloop_controller._vq_set;
    _mt._vf_open_control_law._phase_src = &_VF_openloop_controller._phase;
    _mt._vf_open_control_law._phase_vel_src = &_VF_openloop_controller._phase_vel;

    _mt.arm(&_mt._vf_open_control_law);
}

void Axis::run_current_loop(){

    _mt.update_current_control_law_pi_gain();

    _control._id_set = 0.0f;
    _control._iq_set = 0.2f;

    _mt._current_control_law._id_set_src = &_control._id_set;
    _mt._current_control_law._iq_set_src = &_control._iq_set;
    _mt._current_control_law._vd_set_src = &_mt._vd_set;
    _mt._current_control_law._vq_set_src = &_mt._vq_set;
    _mt._current_control_law._phase_src = &_encoder._phase;
    _mt._current_control_law._phase_vel_src = &_encoder._phase_vel;

    _mt._id_set_src = &_control._id_set;
    _mt._iq_set_src = &_control._iq_set;
    _mt._phase_vel_src = &_encoder._phase_vel;

    _mt.arm(&_mt._current_control_law);
}

void Axis::run_direction_calibration(){
    _VF_openloop_controller._target_vel = 3.0f;
    run_open_loop();

    while(1){
        if( _VF_openloop_controller._total_turn >= 3){
            if(_encoder._phase_vel > 0){
                _encoder._config.direction = 1.0f;
            }else{
                _encoder._config.direction = -1.0f;
            }
            break;
        }
        vTaskDelay(1);
    }
    _VF_openloop_controller._target_vel = 0.0f;
    _mt.disarm();
}

void Axis::run_offset_calibration(){

    _VF_openloop_controller._target_vel = 0.0f;
    _VF_openloop_controller._phase = 0.0f;
    _VF_openloop_controller._phase_vel = 0.0f;
    _VF_openloop_controller._cur_vel = 0.0f;
    _VF_openloop_controller._vd_set = -0.5f;

    run_open_loop();

    vTaskDelay(3000);

    uint8_t i = 0;
    uint32_t offset = 0;
    while(1){
        if( i < 5){
            offset += _encoder._cnt_in_cpr;
            i++;
            if(i == 5){
                _encoder._config.phase_offset_cpr = offset / 5;
                break;
            }
        }
        vTaskDelay(1);
    }
    _mt.disarm();
}

void Axis::run_idle(){
    _mt.disarm();

    for(uint8_t i = 0;i < sizeof(_status_chain[i])/sizeof(_status_chain[0]);i++){
        _status_chain[i] = AXIS_STATUS_UNDEFINE;
    }
}

void Axis::thread_wakeup(){
    BaseType_t xHigherPriorityTaskWoken = 0;

    xSemaphoreGiveFromISR(_sem,&xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void Axis::task(void* ctx){
    Axis* axis = (Axis*)ctx;
    uint8_t i = 0,len = 0;

    while(1){
        if(xSemaphoreTake(axis->_sem,portMAX_DELAY) == pdPASS){

            if(axis->_status != AXIS_STATUS_UNDEFINE){
                
                axis->_status_chain[i] = axis->_status;
                axis->_status = AXIS_STATUS_UNDEFINE;
                if(len != 0){
                    len = 0;
                }
                len++;
            }

            switch(axis->_status_chain[i]){
                case AXIS_STATUS_UNDEFINE:
                    i = 0;
                    break;
                case AXIS_STATUS_IDLE:
                    axis->run_idle();
                    i = 0;
                    break;
                case AXIS_STATUS_DIR_CALIBRATION:
                    axis->run_direction_calibration();
                    axis->_status_chain[i] = AXIS_STATUS_UNDEFINE;
                    i++;
                    break;
                case AXIS_STATUS_OFFSET_CALIBRATION:
                    axis->run_offset_calibration();
                    axis->_status_chain[i] = AXIS_STATUS_UNDEFINE;
                    i++;
                    break;
                case AXIS_STATUS_OPEN_LOOP:
                    axis->run_open_loop();
                    axis->_status_chain[i] = AXIS_STATUS_UNDEFINE;
                    i++;
                    break;
                case AXIS_STATUS_CLOSE_LOOP:
                    axis->run_current_loop();
                    axis->_status_chain[i] = AXIS_STATUS_UNDEFINE;
                    i++;
                    break;
            }
            if(i >= len){
                i = 0;
            }
        }
    }
    

} 