#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <stddef.h>
#include <stdint.h>
#include <cmath>
#include <tuple>
#include "arm_math.h"

#define SQRT3_BY_2      0.8660254f      // √3/2
#define ONE_BY_SQRT3    0.57735027f     // 1/√3
#define TWO_BY_3        0.66666667f     // 2/3
#define SQRT3           1.7320508f      // √3
#define TWO_BY_SQRT3    1.1547005f       // 2/√3


template <typename T>
constexpr T SQ(const T& x){
    return x*x;
}


static inline float wrap_pos(float x,int32_t range){
    if(x >= range){
        x -= range;
    }else if(x < 0){
        x += range;
    }
    return x;
}

static inline int32_t wrap_half(int32_t x,int32_t range){
    if(x > range/2){
        x -= range;
    }else if( x < (-range/2) ){
        x += range;
    }
    return x;
}

static inline float wrap_half(float x,int32_t range){
    if(x > range/2.0f){
        x -= range;
    }else if( x < (-range/2.0f) ){
        x += range;
    }
    return x;
}

static inline int32_t multple_roundf(float x,float y){
    return (int32_t)lroundf(x/y);
}

//归一化到[-pi,pi]
static inline float wrap_pi(float x){

    x = x - multple_roundf(x,2*PI)*2*PI;

    if(x > PI){
        x -= 2*PI;
    }else if( x < (-PI) ){
        x += 2*PI;
    }
    return x;
}


std::tuple<float,float,float,bool> SVPWM(float alpha,float beta);

#endif
