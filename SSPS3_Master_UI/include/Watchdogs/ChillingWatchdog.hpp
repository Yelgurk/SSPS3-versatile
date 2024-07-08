#pragma once
#ifndef ChillingWatchdog_hpp
#define ChillingWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

#define AWAIT_UNTILL_TOGGLE_CHILL_VALVE_SS      2
#define TOGGLE_WJACKET_VALVE_INTERVAL_SS        10

class ChillingWatchdog
{
private:
    function<void(bool)> turn_wJacket_valve;
    bool current_state;
    unsigned long last_control_time;
    unsigned long toggle_start_time;
    bool toggle_state;

public:
    ChillingWatchdog(function<void(bool)> turn_wJacket_valve_func) :
        turn_wJacket_valve(turn_wJacket_valve_func),
        current_state(false),
        last_control_time(0),
        toggle_start_time(0),
        toggle_state(false)
    {}

    void get_aim(double needed_temp, double current_temp) {
        current_state = current_temp > needed_temp;
    }

    void do_control(bool save_water_tech = false)
    {
        unsigned long current_time = millis();

        if (save_water_tech && current_state)
        {
            if (current_time - toggle_start_time >= TOGGLE_WJACKET_VALVE_INTERVAL_SS * 1000)
            {
                toggle_state = !toggle_state;
                turn_wJacket_valve(toggle_state);
                toggle_start_time = current_time;
            }
        }
        else
        {
            if (current_time - last_control_time >= AWAIT_UNTILL_TOGGLE_CHILL_VALVE_SS * 1000)
            {
                turn_wJacket_valve(current_state);
                last_control_time = current_time;
            }
        }
    }
};

#endif