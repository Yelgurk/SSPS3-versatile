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
    bool is_wJacket_step;
    bool is_program_running;

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
    valve_state(false),
    is_wJacket_step(false),
    is_program_running(false)
    {}

    void water_in_jacket(bool state, bool is_program_running, bool is_wJacket_step)
    {
        this->is_wJacket_step = is_wJacket_step;
        this->is_program_running = is_program_running;
        has_water_in_jacket = state;

        if (!is_program_running)
        {
            draining_started = false;
            last_control_time_ms = millis();
        }
    }

    void do_control()
    {
        if (is_program_running)
        {
            uint32_t current_time_ms = millis();

            if (has_water_in_jacket || is_wJacket_step)
            {
                set_pause(draining_started = false);
                last_control_time_ms = current_time_ms;
            }

            if (has_water_in_jacket && valve_state && !is_wJacket_step)
                turn_wJacket_valve(valve_state = false);

            if (!has_water_in_jacket)
                turn_wJacket_valve(valve_state = true);

            if (!has_water_in_jacket && !draining_started && (current_time_ms - last_control_time_ms >= time_span_ss_toggle_wJacket_valve_interval * 1000))
            {
                set_pause(true);
                draining_started = true;
            }
        }
        else
            turn_wJacket_valve(valve_state = false);
    }
};

#endif