#include "protocol.hpp"
#include "board.h"
#include "my_debug.h"
#include "oscilloscope.hpp"
#include "motor.hpp"
#include "encoder.hpp"

#define LEN_IDX             (sizeof(prot_head)/sizeof(prot_head[0]))
#define PAYLOAD_START_IDX   (FRAME_LEN_SIZE + sizeof(prot_head)/sizeof(prot_head[0]))

const uint8_t prot_head[] = {0x5A,0xA5,0x5A,0xA5};

Protocol    my_prot(&usb_protocol_backend);


uint8_t query_handle(uint8_t* buf,uint8_t keyword){
    uint8_t len = 0;

    switch(keyword){
        case 0x01:{
                    Axis::AxisStatus* rd_ptr;
                    uint8_t tmp;

                    rd_ptr = get_axis_status(&axes[0]);

                    tmp = (uint8_t)(*rd_ptr);

                    *buf = U8;
                    memcpy(buf + 1,&tmp,sizeof(uint8_t));
                    len = sizeof(uint8_t) + 1;
                    break;
        }
        case 0x05:{
                    float* rd_ptr;
                    rd_ptr = get_motor_iu_ptr(&mt);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
        }
        case 0x06:{
                    float* rd_ptr;
                    rd_ptr = get_motor_iv_ptr(&mt);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
        }
        case 0x07:{
                    float* rd_ptr;
                    rd_ptr = get_motor_iw_ptr(&mt);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
        }
        case 0x11:{
                    float* rd_ptr;
                    rd_ptr = get_encoder_phase_ptr(&encoder);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
        }
        case 0x12:{
                    float* rd_ptr;
                    rd_ptr = get_encoder_pos_est_ptr(&encoder);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
            }
        case 0x13:{
                    float* rd_ptr;
                    rd_ptr = get_encoder_vel_est_ptr(&encoder);

                    *buf = FLOAT;
                    memcpy(buf + 1,rd_ptr,sizeof(float));
                    len = sizeof(float) + 1;
                    break;
        }
        default:
                    *buf = 0xFF;
                    *(buf+1) = 0xFF;
                    len = 2;
    }

    return len;
}

uint8_t set_handle(uint8_t* buf,uint8_t keyword,uint8_t type){
    uint8_t len = 0;

    switch(keyword){
        case 0x01:{ //axis status
            if(type != U8) return 0xFF;
            len = sizeof(U8); 

            uint8_t status = *buf;

            set_axis_status(&axes[0],(Axis::AxisStatus)status);
            break;
        }
        case 0x20:{
                    float turn_vel;

                    if(type != FLOAT) return 0xFF;
                    len = sizeof(float);

                    memcpy(&turn_vel,buf,len);

                    set_openloop_target_vel(&axes[0],turn_vel);
                    break;
        }
        default:
            return 0xFF;
    } 

    return len;
}

void osc_handle(uint8_t* keyword,uint8_t len){
    float* ptr[3] = {nullptr};

    for(uint8_t i = 0;i < len;i++){
        switch(keyword[i]){
            case 0x05:{
                #if (SAMPLE_I_ADC_ENABLE == 1)
                ptr[i] = get_motor_u_adc_ptr(&mt);
                #else
                ptr[i] = get_motor_iu_ptr(&mt);
                #endif
                break;
            }
            case 0x06:{
                #if (SAMPLE_I_ADC_ENABLE == 1)
                ptr[i] = get_motor_v_adc_ptr(&mt);
                #else
                ptr[i] = get_motor_iv_ptr(&mt);
                #endif
                break;
            }
            case 0x07:{
                #if (SAMPLE_I_ADC_ENABLE == 1)
                ptr[i] = get_motor_w_adc_ptr(&mt);
                #else
                ptr[i] = get_motor_iw_ptr(&mt);
                #endif
                break;
            }

            case 0x10:{
                ptr[i] = get_encoder_phase_ptr(&encoder);
                break;
            }
            case 0x11:{
                ptr[i] = get_encoder_phase_vel_ptr(&encoder);
                break;
            }
            case 0x12:{
                ptr[i] = get_encoder_pos_est_ptr(&encoder);
                break;
            }
            case 0x13:{
                ptr[i] = get_encoder_vel_est_ptr(&encoder);
                break;
            }
            default:
                ptr[i] = nullptr;
                break;
        }
    }

    osc.init(ptr);
    
}

void Protocol::task(void* ctx){

    Protocol* instance = static_cast<Protocol*>(ctx);
    UBaseType_t stack_use_size = 0;

    while(1){
        if(xSemaphoreTake(instance->_sem,portMAX_DELAY) == pdPASS ){
            while(instance->available()){
                instance->read();
                instance->analyse_frame();
                stack_use_size = uxTaskGetStackHighWaterMark(NULL);
            }
            while(instance->_analyse.tx_rd < instance->_analyse.tx_wr){
                size_t size =  instance->_analyse.tx_wr - instance->_analyse.tx_rd;
                uint8_t* wr_ptr = instance->_txbuf + instance->_analyse.tx_rd;

                instance->_analyse.tx_rd += instance->_backend->write(wr_ptr,size);
                
                instance->_backend->flush();
            }
            instance->_analyse.tx_rd = 0;
            instance->_analyse.tx_wr = 0;
        }
    }
    (void)stack_use_size;
}

void Protocol::init(){

    _sem = xSemaphoreCreateBinary();
    xTaskCreate(this->task,"Prot",configMINIMAL_STACK_SIZE*2,this,2,&(this->_task));

    _prot_head = prot_head;
    _prot_head_len = sizeof(prot_head)/sizeof(prot_head[0]);
    _crc_fun = crc8_compute;
    _analyse.status = FOUND_HEAD;
    _analyse.frame_len = 0;
    _analyse.tx_wr = 0;
    _analyse.tx_rd = 0;
}

size_t Protocol::read(){  //从终端读取一次数据到rxbuf
    size_t  size;
    uint8_t* wr_ptr;

    size = _rxbuf.write_contiguous_size();
    wr_ptr = _rxbuf.write_ptr();
    if(size != 0){
        size = _backend->read(wr_ptr,size);
        _rxbuf.advance_wr(size); 
    }else{
        return 0;   //代表队列满了
    }
    return size;    //返回读取长度
}

/*size_t Protocol::write(uint8_t* buf,size_t len){
    size_t  total = 0;

    while(total < len){
        size_t size = _txbuf.write_contiguous_size();
        if(size == 0) break;

        uint8_t* wr_ptr = _txbuf.write_ptr();

        size_t n = (size < (len - total)) ? size:(len - total);
        memcpy(wr_ptr,buf + total,n);
        _txbuf.advance_wr(n);
        total += n;
    }

    return total;   //返回写入总长度
}*/

void Protocol::analyse_frame(){

    uint8_t data;

    while(_rxbuf.empty() == false || _analyse.status > CHECK_SUM){

        if(_analyse.status < PAYLOAD_ANALYSE){
            _rxbuf.pop(data);
        }

        switch(_analyse.status){
            case FOUND_HEAD:
                if(data == _prot_head[_analyse.frame_len]){
                    _analyse.rxframe[_analyse.frame_len++] = data;
                }else{
                    _analyse.frame_len = 0;
                }
                if(_analyse.frame_len == _prot_head_len){
                    _analyse.status = GET_LEN;
                    LOG("protocol: to GET_LEN\n");
                }
                break;
            case GET_LEN:{
                _analyse.payload_len = data;
                if(data + _prot_head_len + FRAME_LEN_SIZE + FRAME_LENE_SIZ <= RX_FRAME_MAX_LEN){
                    _analyse.rxframe[_analyse.frame_len++] = data;
                }else{
                    _analyse.frame_len = 0;
                    _analyse.status = FOUND_HEAD;
                    LOG("protocol: to FOUND_HEAD\n");
                }
                _analyse.status = GET_PAYLOAD;
                LOG("protocol: to GET_PAYLOAD\n");
                break;
            }
            case GET_PAYLOAD:{
                if(_analyse.frame_len < _analyse.payload_len + _prot_head_len + FRAME_LEN_SIZE){
                    _analyse.rxframe[_analyse.frame_len++] = data;
                    if(_analyse.frame_len == _analyse.payload_len + _prot_head_len + FRAME_LEN_SIZE){
                        _analyse.status = CHECK_SUM;
                        LOG("protocol: to CHECK_SUM\n");
                    }
                }
                break;
            }
            case CHECK_SUM:{
                
                uint8_t crc = 0;

                crc = _crc_fun(_analyse.rxframe,_analyse.frame_len);
                
                if(crc == data){
                    _analyse.status = PAYLOAD_ANALYSE;
                    LOG("protocol: to PAYLOAD_ANALYSE\n");
                }else{
                    _analyse.status = FOUND_HEAD;
                    _analyse.frame_len = 0;
                    LOG("protocol: to FOUND_HEAD\n");
                }
                break;
            }
            case PAYLOAD_ANALYSE:
                memcpy(_analyse.txframe,_prot_head,_prot_head_len);
                _analyse.tx_wr = _prot_head_len;
                _analyse.tx_wr++; //跳过长度字节
                if(_analyse.rxframe[PAYLOAD_START_IDX] == PROT_QUERY){
                    _analyse.txframe[_analyse.tx_wr++] = REPLY_QUERY;
                    _analyse.status = QUERY_COMMAND;
                    LOG("protocol: to QUERY_COMMAND\n");
                }else if(_analyse.rxframe[PAYLOAD_START_IDX] == PROT_SET){
                    _analyse.txframe[_analyse.tx_wr++] = REPLY_SET;
                    _analyse.status = SET_COMMAND;
                    LOG("protocol: to SET_COMMAND\n");
                }else if(_analyse.rxframe[PAYLOAD_START_IDX] == PROT_OSC){
                    _analyse.txframe[_analyse.tx_wr++] = REPLY_OSC;
                    _analyse.status = OSC_COMMAND;
                    LOG("protocol: to OSC_COMMAND\n");
                }
                _analyse.payload_len--; //减去命令字占用的1字节
                break;
            case QUERY_COMMAND:{
                for(uint8_t i = 0; i < _analyse.payload_len;i++){
                    _analyse.txframe[_analyse.tx_wr++] = _analyse.rxframe[PAYLOAD_START_IDX + 1 + i];
                    _analyse.tx_wr += query_handle(_analyse.txframe + _analyse.tx_wr,\
                                                        _analyse.rxframe[PAYLOAD_START_IDX + 1 + i]);
                    
                }
                _analyse.txframe[LEN_IDX] =  _analyse.tx_wr - PAYLOAD_START_IDX;
                _analyse.txframe[_analyse.tx_wr] = _crc_fun(_analyse.txframe,_analyse.tx_wr);
                _analyse.tx_wr++;

                //一帧分析好后，直接发送给USB
                _backend->write(_analyse.txframe,_analyse.tx_wr);
                _backend->flush();

                _analyse.status = FOUND_HEAD;
                _analyse.frame_len = 0;
                _analyse.tx_wr = 0;
                break;
            }
            case SET_COMMAND:{
                uint8_t keyword = 0xFF,type = 0xFF,increment = 0;

                for(uint8_t i = 0; i < _analyse.payload_len;i++){
                    if(keyword == 0xFF){
                        keyword = _analyse.rxframe[PAYLOAD_START_IDX + 1 + i];
                        _analyse.txframe[_analyse.tx_wr++] = keyword;
                    }else if(type == 0xFF){
                        type = _analyse.rxframe[PAYLOAD_START_IDX + 1 + i];
                        increment = set_handle(_analyse.rxframe + PAYLOAD_START_IDX + 2 + i,\
                                                                                 keyword,type);

                        _analyse.txframe[_analyse.tx_wr++] = BOOL;
                        if(increment > 4){
                            _analyse.txframe[_analyse.tx_wr++] = false;
                            _analyse.txframe[_analyse.tx_wr++] = increment;
                            switch(type){
                                case BOOL:
                                    increment = sizeof(bool);
                                    break;
                                case U8:
                                    increment = sizeof(uint8_t);
                                    break;
                                case U16:
                                    increment = sizeof(uint16_t);
                                    break;
                                case U32:
                                    increment = sizeof(uint32_t);
                                    break;
                                case FLOAT:
                                    increment = sizeof(float);
                                    break;
                            }
                        }else{
                            _analyse.txframe[_analyse.tx_wr++] = true;
                        }
                        i += increment;
                        keyword = 0xFF;
                        type = 0xFF;
                    }
                }
                _analyse.txframe[LEN_IDX] = _analyse.tx_wr - PAYLOAD_START_IDX;
                _analyse.txframe[_analyse.tx_wr] = _crc_fun(_analyse.txframe,_analyse.tx_wr);
                _analyse.tx_wr++;

                //一帧分析好后，直接发送给USB
                _backend->write(_analyse.txframe,_analyse.tx_wr);
                _backend->flush();
                
                _analyse.status = FOUND_HEAD;
                _analyse.frame_len = 0;
                _analyse.tx_wr = 0;
                break;
            }
            case OSC_COMMAND:{
                size_t data_len_idx = 0,data_len = 0;

                for(uint8_t i = 0; i < _analyse.payload_len;i++){
                    _analyse.txframe[_analyse.tx_wr++] = _analyse.rxframe[PAYLOAD_START_IDX + 1 + i];
                }
                osc_handle(&_analyse.rxframe[PAYLOAD_START_IDX + 1],_analyse.payload_len);    

                _analyse.txframe[LEN_IDX] =  _analyse.tx_wr - PAYLOAD_START_IDX;
                _analyse.txframe[_analyse.tx_wr] = _crc_fun(_analyse.txframe,_analyse.tx_wr);
                _analyse.tx_wr++;
                memcpy(_txbuf,_analyse.txframe,_analyse.tx_wr);
                data_len_idx = _analyse.tx_wr;
                _analyse.tx_wr += 2;    //跳过2个字节的数据长度
                
                while(osc.done() == false){
                    vTaskDelay(1);  //1ms等待
                }
                _analyse.tx_wr += osc.read(_txbuf+_analyse.tx_wr);
                data_len = _analyse.tx_wr - data_len_idx - 2;
                memcpy(_txbuf+data_len_idx,&data_len,2);

                _analyse.status = FOUND_HEAD;
                _analyse.frame_len = 0;
                break;
            }
        };
    }
}

void my_protocol_init(){
    my_prot.init();
}

Protocol& get_my_prot(){
    return my_prot.get();
}


