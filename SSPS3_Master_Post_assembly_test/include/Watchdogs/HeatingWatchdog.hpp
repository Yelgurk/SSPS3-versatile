#pragma once
#ifndef HeatingWatchdog_hpp
#define HeatingWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

#define AWAIT_UNTILL_TOGGLE_HEATERS_SS  20

class HeatingWatchdog
{
private:
    function<void(bool)> turn_heaters;
    bool current_state;
    unsigned long last_control_time;

public:
    HeatingWatchdog(function<void(bool)> turn_heaters_func) :
    turn_heaters(turn_heaters_func),
    current_state(false),
    last_control_time(0)
    {}

    void get_aim(double needed_temp, double current_temp1) {
        current_state = current_temp1 < needed_temp;
    }

    void get_aim(double needed_temp, double current_temp1, double current_temp2)
    {
        double max_temp = (current_temp1 > current_temp2) ? current_temp1 : current_temp2;
        current_state = max_temp < needed_temp;
    }

    void do_control()
    {
        unsigned long current_time = millis();

        if (current_time - last_control_time >= AWAIT_UNTILL_TOGGLE_HEATERS_SS * 1000)
        {
            turn_heaters(current_state);
            last_control_time = current_time;
        }
    }
};

#endif