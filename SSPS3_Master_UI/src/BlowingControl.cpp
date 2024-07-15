#include "../include/BlowingControl.hpp"

void BlowingControl::blowgun_trigger(bool do_gurgling, bool is_keypad_press, int8_t index, BlowgunValue curr_value)
{
    if (!do_gurgling)
        trigger_must_be_reloaded = false;

    uint32_t current_time = millis();
    
    if (do_gurgling && !trigger_must_be_reloaded)
    {
        if (is_runned && current_blow_index != index)
        {
            stop_pump();
            timer_running = false;
            return;
        }

        if (!is_runned && is_keypad_press)
            callback(curr_value.val * 1000, 0.f, curr_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, 0.001f);

        if ((!timer_running || !is_runned) && ((is_keypad_press && current_time - last_call_time < 2000) || !is_keypad_press))
        {
            if (!is_runned)
            {
                selected_blowing_value = curr_value;
                current_blow_index = index;
                is_runned = true;

                ml_per_ms = pump_power_lm * 1000.f / 60.f / 1000.f;

                if (!selected_blowing_value.is_timer)
                    ms_aim = (curr_value.val / (pump_power_lm * 1000)) * (60 * 1000);
                else
                    ms_aim = curr_value.val * 1000;

                ms_gone = 0;
            }

            last_blow_time = current_time;
            timer_running = true;
            start_pump();
        } 

        last_call_time = current_time;
    }
    else
    {
        stop_pump();
        timer_running = false;
    }
}

void BlowingControl::blowgun_stop(bool need_in_reload)
{
    stop_pump();

    if (need_in_reload)
        trigger_must_be_reloaded = true;

    timer_running = false;
    is_runned = false;
    callback(0, 0, BlowingType::LITER, 0);
}

void BlowingControl::do_blowing()
{
    if (!is_runned) return;

    uint32_t current_time = millis();

    if (current_time - last_call_time > 2000)
    {
        blowgun_stop();
        return;
    }

    if (timer_running)
    {
        ms_gone += current_time - last_blow_time;
        last_blow_time = current_time;

        callback(ms_aim, ms_gone, selected_blowing_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);

        if (ms_gone >= ms_aim)
        {
            callback(ms_aim, ms_gone, selected_blowing_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);
            blowgun_stop(true);
        }
    }
}

void BlowingControl::start_pump()
{
    pump_on = true;
}

void BlowingControl::stop_pump()
{
    pump_on = false;
}