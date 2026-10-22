#ifndef __COMMUNICATIE_COMMON_H__
#define __COMMUNICATIE_COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#if __cplusplus
class Backend {
public:

    virtual bool connected() = 0;

    virtual uint32_t available() = 0;

    virtual uint32_t read(void* buffer, uint32_t bufsize) = 0;

    virtual uint32_t write(void const* buf, uint32_t size)= 0;

    virtual uint32_t flush() = 0;

    virtual void init() {}
};

template<typename T,size_t N>
class RingBuf {
    static_assert((N&(N-1)) == 0);
public:
    
    uint8_t& operator[](uint8_t idx){
        return _buf[idx];
    }

    bool full() const {return _cnt == N;}

    bool empty()const {return _cnt == 0;};

    bool push(T v) {_buf[_wr] = v;_wr=wrap(_wr+1);++_cnt;return true;}

    bool pop(T& v) {v = _buf[_rd];_rd=wrap(_rd+1);--_cnt;return true;}

    size_t write_contiguous_size(){
        if(full()){
            return 0;
        }
        size_t free = N - _cnt;
        size_t to_end = N - _wr;
        return to_end < free ? to_end:free;
    }

    T* write_ptr() {return _buf+_wr;}
    
    void advance_wr(size_t n) { _wr = wrap(_wr+n); _cnt += n; }

    size_t read_contiguous_size(){
        if(empty()){
            return 0;
        }
        size_t to_end = N - _rd;
        return to_end < _cnt? to_end:_cnt;
    }

    T* read_ptr() {return _buf+_rd;}

    void advance_rd(size_t n) {_rd = wrap(_rd+n); _cnt -= n;}

    static size_t wrap(size_t i) {return i & (N-1);} //回绕,等效取余

    size_t size() {return _cnt;};

private:
    T       _buf[N];
    size_t  _cnt = 0;
    size_t  _wr = 0;
    size_t  _rd = 0;
};
#endif

#endif
