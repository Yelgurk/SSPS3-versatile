#include "../include/BlowingControl.hpp"

void BlowgunControl::blowgun_trigger(bool do_gurgling, bool is_keypad_press, int8_t index = -1, BlowgunValue curr_value = BlowgunValue())
{
    if (!do_gurgling)
        trigger_must_be_reloaded = false;

    uint32_t current_time = millis();
    
    if (do_gurgling && !trigger_must_be_reloaded)
    {
        if (is_active && current_blow_index != index)
        {
            stop_pump();
            timer_running = false;
            return;
        }

        if (!is_active && is_keypad_press)
            callback(curr_value.val * 1000, 0.f, curr_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, 0.001f);

        if ((!timer_running || !is_active) && ((is_keypad_press && current_time - last_call_time < 2000) || !is_keypad_press))
        {
            if (!is_active)
            {
                selected_blowing_value = curr_value;
                current_blow_index = index;
                is_active = true;

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

void BlowgunControl::blowgun_stop(bool need_in_reload = false)
{
    stop_pump();

    if (need_in_reload)
        trigger_must_be_reloaded = true;

    timer_running = false;
    is_active = false;
    callback(0, 0, BlowingType::LITER, 0);
}

void BlowgunControl::do_blowing()
{
    if (!is_active) return;

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

void BlowgunControl::start_pump()
{
    pump_on = true;
}

void BlowgunControl::stop_pump()
{
    pump_on = false;
}