#ifndef __CONTROL_HPP__
#define __CONTROL_HPP__

class Axis;


class Control{
public:
        Control(){}

        float _id_set = 0.0f;
        float _iq_set = 0.5f;

        Axis* _axis = nullptr;
};

#endif
