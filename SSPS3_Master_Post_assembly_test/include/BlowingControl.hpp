#pragma once

#ifndef BlowingControl_hpp
#define BlowingControl_hpp

#include <Arduino.h>
#include "../include/UIControls/UIBlowingControl.hpp"

struct BlowgunValue
{
    uint16_t val = 5000;
    bool is_timer = false;

    BlowgunValue() {}
    BlowgunValue(uint16_t val) : val(val) {}
    BlowgunValue(bool is_timer, uint16_t val) : is_timer(is_timer), val(val) {}
};

class BlowingControl
{
public:
    bool timer_running;
    bool is_runned;
    using CallbackFunc = std::function<void(float, float, BlowingType, float)>;

private:
    BlowgunValue selected_blowing_value;
    CallbackFunc callback;
    int8_t current_blow_index = -1;

    uint32_t ms_aim = 0;
    uint32_t ms_gone = 0;
    float ml_per_ms = 0;
    float pump_power_lm = 52.f;

    bool trigger_must_be_reloaded = false;
    bool pump_on;
    uint32_t last_call_time;
    uint32_t last_blow_time;

    void start_pump();
    void stop_pump();

public:
    BlowingControl() :
    is_runned(false),
    last_call_time(0), 
    pump_on(false),
    timer_running(false)
    {}

    void set_blowing_ui_callback(CallbackFunc callback) {
        this->callback = callback;
    }

    void blowgun_trigger(bool do_gurgling, bool is_keypad_press, int8_t index = -1, BlowgunValue curr_value = BlowgunValue());
    void blowgun_stop(bool need_in_reload = false);
    void do_blowing();
};

#endif