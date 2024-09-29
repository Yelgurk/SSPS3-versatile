#pragma once
#ifndef HeatingWatchdog_hpp
#define HeatingWatchdog_hpp

#include <Arduino.h>
#include <functional>

using namespace std;

class HeatingWatchdog
{
private:
    function<void(bool)> turn_heaters;
    bool current_state;
    uint32_t last_control_time_ms;
    uint16_t time_span_ss_toggle_heaters_interval;
    uint8_t wJacket_max_tempC;

public:
    HeatingWatchdog(
        function<void(bool)> turn_heaters_func,
        uint16_t time_span_ss_toggle_heaters_interval,
        uint8_t wJacket_max_tempC
    ) :
    turn_heaters(turn_heaters_func),
    time_span_ss_toggle_heaters_interval(time_span_ss_toggle_heaters_interval),
    wJacket_max_tempC(wJacket_max_tempC),
    current_state(false),
    last_control_time_ms(0)
    {}

    void set_aim(double needed_temp, double product_tempC) {
        current_state = product_tempC < needed_temp;
    }

    void set_aim(double needed_temp, double product_tempC, double wJacket_tempC) {
        current_state = wJacket_tempC >= wJacket_max_tempC ? false : (product_tempC < needed_temp);
    }

    void emergency_stop()
    {
        turn_heaters(current_state = false);
        last_control_time_ms = millis();
    }

    void do_control()
    {
        uint32_t current_time_ms = millis();

        if (current_time_ms - last_control_time_ms >= time_span_ss_toggle_heaters_interval * 1000)
        {
            turn_heaters(current_state);
            last_control_time_ms = current_time_ms;
        }
    }
};

#endif