#pragma once
#ifndef AsyncMotorWD_hpp
#define AsyncMotorWD_hpp

#include <Arduino.h>
#include <functional>
#include <cmath>

#define ADC_MAX             (uint16_t)4095
#define MCU_MAX_VOLTAGE     10.f

using namespace std;

class AsynchronousMotorWatchdog
{
private:
    function<void(bool)> turn_motor_func;
    function<void(uint8_t)> set_motor_speed_func;
    uint8_t max_rot_per_min;

public:
    AsynchronousMotorWatchdog(
        std::function<void(bool)> turn_motor_func, 
        std::function<void(uint8_t)> set_motor_speed_func, 
        uint8_t max_rot_per_min
    ) :
    turn_motor_func(turn_motor_func),
    set_motor_speed_func(set_motor_speed_func),
    max_rot_per_min(max_rot_per_min)
    {}

    void set_async_motor_speed(uint8_t rot_per_min)
    {
        turn_motor_func(rot_per_min > 0);
        set_motor_speed_func(min(rot_per_min, max_rot_per_min));
    }
};

#endif