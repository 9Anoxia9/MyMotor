#include "oscilloscope.hpp"
#include "string.h"


void Osc::init(float** data_src){

    for(uint8_t i = 0;i < 3;i++){
        _data_src[i] = data_src[i];
        if(_data_src[i] != nullptr){
            _active_ch_num++;
        }
        _cnt[i] = 0;
    }
}

void Osc::update(){

    for(uint8_t i = 0;i < 3; i++){
        if(_data_src[i] == nullptr){
            continue;
        }
        if(_cnt[i] < CHANNEL_DATA_SIZE){
            _sample_data[i][_cnt[i]] = (*_data_src[i]);
            _cnt[i] +=1;
        }
    }
}

/**
 * 需要保证外部wr_ptr足够大，能够容纳所有数据。
 */
size_t Osc::read(uint8_t* wr_ptr){
    size_t len = 0;

    for(uint8_t i = 0;i < 3; i++){
        if(_data_src[i] != nullptr){
            memcpy(wr_ptr+(i*CHANNEL_DATA_SIZE_U8),&_sample_data[i],CHANNEL_DATA_SIZE_U8);
            len += CHANNEL_DATA_SIZE_U8;
        }
    }
    return len;
}

bool Osc::done(){
    bool breturn = false;
    uint8_t full_ch_num = 0;

    for(uint8_t i = 0;i < _active_ch_num; i++){
        if(_cnt[i] != CHANNEL_DATA_SIZE){
            break;
        }else{
            full_ch_num++;
        }
    }
    if(full_ch_num == _active_ch_num){
        _active_ch_num = 0;
        breturn = true;
    }
    return breturn;
}