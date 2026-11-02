#ifndef __OSCILLSCOPE_HPP__
#define __OSCILLSCOPE_HPP__

#include <stdint.h>
#include <stddef.h>

#define CHANNEL_NUM             3
#define CHANNEL_DATA_SIZE       256
#define CHANNEL_DATA_SIZE_U8    (CHANNEL_DATA_SIZE<<2)

class Osc {
public:

    void init(float** data_src);
    void update();
    size_t read(uint8_t* wr_ptr);
    bool done();

    float                _sample_data[CHANNEL_NUM][CHANNEL_DATA_SIZE];
    volatile float*      _data_src[CHANNEL_NUM] = {nullptr};
    volatile uint16_t    _cnt[CHANNEL_NUM];
    volatile uint8_t     _active_ch_num = 0;
    volatile bool        _done = false;
};

#endif
