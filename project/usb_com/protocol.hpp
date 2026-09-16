#ifndef __PROTOCOL_HPP__
#define __PROTOCOL_HPP__


#include "interface_usb.hpp"


#define PROTOCOL_BUF_SIZE       256//OSC_DATA_SIZE_MAX
#define OSC_DATA_SIZE_MAX       4096


#define FRAME_LEN_SIZE      1
#define FRAME_LENE_SIZ      1
#define RX_FRAME_MAX_LEN    16

typedef enum command_word{
    PROT_QUERY  =   0x01,
    PROT_SET    =   0x02,
    REPLY_QUERY =   0x81,
    REPLY_SET   =   0x82,
    PROT_OSC    =   0x03,
    REPLY_OSC   =   0x83  
}command;

typedef enum Data_Type{
    BOOL = 0,
    U8,
    U16,
    U32,
    FLOAT, 
}dtype;


class Protocol{
public:
        typedef enum Analyse_status{
            FOUND_HEAD = 0,
            GET_LEN,
            GET_PAYLOAD,
            CHECK_SUM,
            PAYLOAD_ANALYSE,
            QUERY_COMMAND,
            SET_COMMAND,
            OSC_COMMAND
        }A_Status;

        typedef struct{
            uint8_t     frame_len;                      //帧长度,也是rx_frame的索引
            uint8_t     payload_len;                    //信息长度
            uint8_t     rxframe[RX_FRAME_MAX_LEN];      //存储一帧数据
            uint8_t     txframe[RX_FRAME_MAX_LEN<<1];   //存储一帧数据
            size_t      tx_wr;                          //tx写索引，也是帧长度
            size_t      tx_rd;                          //tx读索引
            A_Status    status;         //分析状态机
        }FrameData;

        using crc_handle = uint8_t (*)(uint8_t* buf,uint8_t len);

public:
        Protocol(Backend* backend):_backend(backend){}

        Protocol& get(){
            return *this;
        }

        void init();

        size_t read();

        size_t write(uint8_t* buf,size_t len);

        size_t available(){
            return _backend->available();
        }

        size_t flush(){
            return _backend->flush();
        }

        void analyse_frame();

        static void task(void* ctx);

        FrameData           _analyse;

        SemaphoreHandle_t   _sem;
        TaskHandle_t        _task;
        
private:
        Backend*            _backend;

        RingBuf<uint8_t,PROTOCOL_BUF_SIZE>      _rxbuf;
        uint8_t             _txbuf[OSC_DATA_SIZE_MAX];  //该缓冲区仅用于存放示波器指令及其数据包

        crc_handle          _crc_fun;

        const uint8_t*      _prot_head; 
        uint8_t             _prot_head_len;
};


void my_prot_init();
Protocol& get_my_prot();

#endif
