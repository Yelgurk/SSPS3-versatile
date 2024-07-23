#pragma once

#ifndef BlowingControl_hpp
#define BlowingControl_hpp

#include <Arduino.h>
#include "../include/UIControls/UIBlowingControl.hpp"
#include "../include/UIControls/UINotificationBar.hpp"
#include "STM32Pinouts.hpp"

struct __attribute__((packed)) BlowgunValue
{
    uint16_t val = 5000;
    bool is_timer = false;

    BlowgunValue() {}
    BlowgunValue(uint16_t val) : val(val) {}
    BlowgunValue(bool is_timer, uint16_t val) : is_timer(is_timer), val(val) {}
};

enum class BlowingTriggerType : uint8_t {
    NONE,
    KEYBOARD,
    PISTOL
};

extern uint8_t      OptIn_state[8];

class BlowingControl
{
public:
    BlowingTriggerType triggered_by = BlowingTriggerType::NONE;
    bool timer_running;
    bool is_runned;
    using CallbackFunc = std::function<void(float, float, BlowingType, float)>;
    using ControlBlowingFunc = std::function<void(bool)>;

private:
    UINotificationBar * UI_notification_bar;
    CallbackFunc callback;
    ControlBlowingFunc blowing_func;

    BlowgunValue selected_blowing_value;
    int8_t current_blow_index = -1;

    uint32_t ms_aim = 0;
    uint32_t ms_gone = 0;
    float ml_per_ms = 0;

    float pump_power_lm;
    uint8_t time_span_ss_on_pause_await_limit;

    bool trigger_must_be_reloaded = false;
    bool keyboard_must_be_reloaded = false;
    bool pump_on;
    uint32_t last_call_time_ms;
    uint32_t last_blow_time_ms;

    void start_pump();
    void stop_pump();

public:
    BlowingControl(
        UINotificationBar * UI_notification_bar,
        ControlBlowingFunc blowing_func,
        uint8_t time_span_ss_on_pause_await_limit,
        float var_blowing_pump_power_lm
    ) :
    UI_notification_bar(UI_notification_bar),
    blowing_func(blowing_func),
    pump_power_lm(var_blowing_pump_power_lm),
    time_span_ss_on_pause_await_limit(time_span_ss_on_pause_await_limit),
    is_runned(false),
    last_call_time_ms(0), 
    pump_on(false),
    timer_running(false)
    {}

    void set_blowing_ui_callback(CallbackFunc callback) {
        this->callback = callback;
    }

    void blowgun_trigger(bool do_gurgling, bool is_keypad_press, float blowing_calibration_value, int8_t index = -1, BlowgunValue curr_value = BlowgunValue());
    void blowgun_stop(bool need_in_reload = false);
    void do_blowing();
};

#endif