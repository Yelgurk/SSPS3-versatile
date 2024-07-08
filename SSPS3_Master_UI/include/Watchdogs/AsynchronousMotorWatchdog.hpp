#pragma once
#ifndef AsyncMotorWD_hpp
#define AsyncMotorWD_hpp

#include <Arduino.h>
#include <functional>
#include <cmath>

using namespace std;

class AsynchronousMotorWatchdog
{
private:
    function<void(bool)> turn_motor_func;
    function<void(uint16_t)> set_motor_speed_func;
    uint8_t max_rot_per_min;
    float max_out_voltage;
    float voltage_scale;

    static const uint16_t ADC_MAX = 4095;
    static const float MCU_MAX_VOLTAGE = 10.0;

public:
    AsynchronousMotorWatchdog(
        std::function<void(bool)> turn_motor_func, 
        std::function<void(uint16_t)> set_motor_speed_func, 
        uint8_t max_rot_per_min, 
        float max_out_voltage
    ) :
    turn_motor_func(turn_motor_func),
    set_motor_speed_func(set_motor_speed_func),
    max_rot_per_min(max_rot_per_min),
    max_out_voltage(max_out_voltage)
    {
        voltage_scale = max_out_voltage / MCU_MAX_VOLTAGE;
    }

    void turn_async_motor(bool state) {
        turn_motor_func(state);
    }

    void set_async_motor_speed(uint8_t rot_per_min)
    {
        rot_per_min = std::min(rot_per_min, max_rot_per_min);
        set_motor_speed_func(static_cast<uint16_t>(round(
            (static_cast<float>(rot_per_min) / max_rot_per_min)
            * voltage_scale
            * ADC_MAX
        )));
    }
};

#endif