#pragma once
#ifndef WaterJacketDrainWatchdog_hpp
#define WaterJacketDrainWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

class WaterJacketDrainWatchdog
{
private:
    function<void(bool)> turn_wJacket_valve;
    function<void(bool)> set_pause;
    uint16_t time_span_ss_toggle_wJacket_valve_interval;
    uint32_t last_control_time_ms;
    bool has_water_in_jacket;
    bool draining_started;
    bool valve_state;

public:
    WaterJacketDrainWatchdog(
        function<void(bool)> turn_wJacket_valve_func, function<void(bool)> set_pause_func,
        uint16_t time_span_ss_toggle_wJacket_valve_interval
    ) :
    turn_wJacket_valve(turn_wJacket_valve_func),
    set_pause(set_pause_func),
    time_span_ss_toggle_wJacket_valve_interval(time_span_ss_toggle_wJacket_valve_interval),
    last_control_time_ms(0),
    has_water_in_jacket(false),
    draining_started(false),
    valve_state(false)
    {}

    void water_in_jacket(bool state)
    {
        if (!has_water_in_jacket && state)
        {
            set_pause(false);
            turn_wJacket_valve(valve_state = false);
        }

        if (has_water_in_jacket = state)
            draining_started = false;
    }

    void do_control()
    {
        uint32_t current_time_ms = millis();

        if (has_water_in_jacket)
            last_control_time_ms = current_time_ms;
        else
        {
            if (!valve_state)
                turn_wJacket_valve(valve_state = true);

            if (!draining_started && (current_time_ms - last_control_time_ms >= time_span_ss_toggle_wJacket_valve_interval * 1000))
            {
                set_pause(true);
                draining_started = true;
            }
        }
    }
};

#endif