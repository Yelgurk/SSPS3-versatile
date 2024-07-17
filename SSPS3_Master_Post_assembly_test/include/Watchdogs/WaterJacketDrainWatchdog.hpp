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
    bool has_water_in_jacket;
    unsigned long last_control_time;
    unsigned long last_drain_time;
    bool draining_started;

public:
    WaterJacketDrainWatchdog(function<void(bool)> turn_wJacket_valve_func, function<void(bool)> set_pause_func) :
    turn_wJacket_valve(turn_wJacket_valve_func),
    set_pause(set_pause_func),
    has_water_in_jacket(false),
    last_control_time(0),
    last_drain_time(0),
    draining_started(false)
    {}

    void set_has_water_in_jacket(bool state)
    {
        has_water_in_jacket = state;

        if (has_water_in_jacket)
        {
            last_control_time = millis();

            draining_started = false;
            turn_wJacket_valve(false);
        }
    }

    void do_control()
    {
        unsigned long current_time = millis();

        if (has_water_in_jacket)
        {
            last_control_time = current_time;
            set_pause(false);
        }
        else
        {
            if (!draining_started && (current_time - last_control_time >= 30000))
            {
                set_pause(true);
                draining_started = true;
                last_drain_time = current_time;
            }

            if (draining_started)
            {
                if (current_time - last_drain_time >= 500)
                {
                    turn_wJacket_valve(true);
                    last_drain_time = current_time;
                }
            }
        }
    }
};

#endif