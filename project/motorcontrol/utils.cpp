#include "utils.hpp"

std::tuple<float,float,float,bool> SVPWM(float alpha,float beta){
    float tU,tV,tW = 0;
    bool success = false;
    uint8_t sector;

    if(beta >= 0.0f){
        if(alpha >= 0.0f){
            if(beta > SQRT3*alpha){
                sector = 2;
            }else{
                sector = 1;
            }
        }else{
            if(beta > (-SQRT3*alpha)){
                sector = 2;
            }else{
                sector = 3;
            }
        }
    }else{
        if(alpha >= 0.0f){
            if(beta < (-SQRT3*alpha)){
                sector = 5;
            }else{
                sector = 6;
            }
        }else{
            if(beta < SQRT3*alpha){
                sector = 5;
            }else{
                sector = 4;
            }
        }
    }

    switch(sector){
        case 1:{
                float t1,t2;
                t1 = alpha - ONE_BY_SQRT3*beta;
                t2 = TWO_BY_SQRT3*beta;

                tU = (1 - t1 - t2)/2;
                tV = tU + t1;
                tW = tV + t2;
                break;
        }
        case 2:{
                float t2,t3;
                t2 = alpha + ONE_BY_SQRT3*beta;
                t3 = -alpha + ONE_BY_SQRT3*beta;

                tV = (1 - t2 - t3)/2;
                tU = tV + t3;
                tW = tU + t2;
                break;
        }
        case 3:{
                float t3,t4;
                t3 = TWO_BY_SQRT3*beta;
                t4 = -alpha - ONE_BY_SQRT3*beta;

                tV = (1 - t3 - t4)/2;
                tW = tV + t3;
                tU = tW + t4;
                break;
        }
        case 4:{
                float t4,t5;
                t4 = -alpha + ONE_BY_SQRT3*beta;
                t5 = -TWO_BY_SQRT3*beta;

                tW = (1 - t4 - t5)/2;
                tV = tW + t5;
                tU = tV + t4;
                break;
        }
        case 5:{
                float t5,t6;
                t5 = - alpha - ONE_BY_SQRT3*beta;
                t6 = alpha - ONE_BY_SQRT3*beta;

                tW = (1 - t5 - t6)/2;
                tU = tW + t5;
                tV = tU + t6;
                break;
        }
        case 6:{
                float t6,t1;
                t6 = -TWO_BY_SQRT3*beta;
                t1 = alpha + ONE_BY_SQRT3*beta;

                tU = (1 - t6 - t1)/2;
                tW = tU + t1;
                tV = tW + t6;
                break;
        }
    }

    success = tU >= 0.0f && tU < 1.0f
                && tV >= 0.0f && tV < 1.0f
                && tW >= 0.0f && tW < 1.0f;

    return {tU,tV,tW,success};
}