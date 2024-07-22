#pragma once
#ifndef ChillingWatchdog_hpp
#define ChillingWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

class ChillingWatchdog
{
private:
    function<void(bool)> turn_wJacket_valve;
    bool expected_state;
    bool toggle_state;
    uint32_t last_control_time_ms;
    
    uint16_t time_span_ss_toggle_wJacket_valve_interval_water_safe_mode = 10;
    uint16_t time_span_ss_toggle_wJacket_valve_interval_simple_mode = 1;
    bool is_water_safe_mode = false;

public:
    ChillingWatchdog(
        function<void(bool)> turn_wJacket_valve_func,
        uint16_t time_span_ss_safe_mode,
        bool is_water_safe_mode
    ) :
        turn_wJacket_valve(turn_wJacket_valve_func),
        expected_state(false),
        last_control_time_ms(0),
        toggle_state(false),
        time_span_ss_toggle_wJacket_valve_interval_water_safe_mode(time_span_ss_safe_mode),
        time_span_ss_toggle_wJacket_valve_interval_simple_mode(1),
        is_water_safe_mode(is_water_safe_mode)
    {}

    void set_aim(double needed_temp, double current_temp) {
        expected_state = current_temp > needed_temp;
    }

    void do_control(bool control_inst = false)
    {
        uint32_t current_time_ms = millis();
        
        uint32_t toggle_interval_ss = 
            is_water_safe_mode
            ? time_span_ss_toggle_wJacket_valve_interval_water_safe_mode
            : time_span_ss_toggle_wJacket_valve_interval_simple_mode;

        if (control_inst || current_time_ms - last_control_time_ms >= toggle_interval_ss * 1000)
        {
            toggle_state =
                is_water_safe_mode
                ? (expected_state ? !toggle_state : expected_state) 
                : expected_state;

            turn_wJacket_valve(toggle_state);

            last_control_time_ms = current_time_ms;
        }
    }
};

#endif