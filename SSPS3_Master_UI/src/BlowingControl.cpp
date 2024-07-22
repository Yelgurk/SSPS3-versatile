#include "../include/BlowingControl.hpp"

void BlowingControl::blowgun_trigger(bool do_gurgling, bool is_keypad_press, int8_t index, BlowgunValue curr_value)
{
    if (!do_gurgling && !is_runned)
    {
        if (!is_keypad_press)
            trigger_must_be_reloaded = false;
        else
            keyboard_must_be_reloaded = false;
    }

    uint32_t current_time_ms = millis();

    if (do_gurgling && ((!trigger_must_be_reloaded && !is_keypad_press) || (!keyboard_must_be_reloaded && is_keypad_press)))
    {
        if (is_runned && current_blow_index != index)
        {
            stop_pump();
            timer_running = false;
            return;
        }

        if (!is_runned && is_keypad_press)
            callback(curr_value.val * 1000, 0.f, curr_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, 0.001f);

        if ((!timer_running || !is_runned) && ((is_keypad_press && current_time_ms - last_call_time_ms < 2000) || !is_keypad_press))
        {
            if (!is_runned)
            {
                selected_blowing_value = curr_value;
                current_blow_index = index;
                is_runned = true;
                triggered_by = is_keypad_press ? BlowingTriggerType::KEYBOARD : BlowingTriggerType::PISTOL;

                ml_per_ms = pump_power_lm / 60.f; // in 1ss => 1000ms i.e. 1l => 1000ml. we store power in l/m, that why after casting we would be do *1000ml and then /1000ms, that why we just do lm/60s

                if (!selected_blowing_value.is_timer)
                    ms_aim = (curr_value.val / (pump_power_lm * 1000)) * (60 * 1000);
                else
                    ms_aim = curr_value.val * 1000;

                ms_gone = 0;
            }

            last_blow_time_ms = current_time_ms;
            timer_running = true;
            start_pump();
        } 

        last_call_time_ms = current_time_ms;
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
    {
        if (triggered_by == BlowingTriggerType::PISTOL)
            trigger_must_be_reloaded = true;
        else
        if (triggered_by == BlowingTriggerType::KEYBOARD)
            keyboard_must_be_reloaded = true;
    }

    timer_running = false;
    is_runned = false;
    triggered_by = BlowingTriggerType::NONE;
    callback(0, 0, BlowingType::LITER, 0);
}

void BlowingControl::do_blowing()
{
    if (!is_runned) return;

    uint32_t current_time_ms = millis();

    if (current_time_ms - last_call_time_ms > time_span_ss_on_pause_await_limit * 1000)
    {
        blowgun_stop();
        return;
    }

    if (timer_running)
    {
        ms_gone += current_time_ms - last_blow_time_ms;
        last_blow_time_ms = current_time_ms;

        callback(ms_aim, ms_gone, selected_blowing_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);

        if (ms_gone >= ms_aim)
        {
            callback(ms_aim, ms_gone, selected_blowing_value.is_timer ? BlowingType::TIMER : BlowingType::LITER, ml_per_ms);
            blowgun_stop(true);
        }
    }
}

void BlowingControl::start_pump() {
    blowing_func(true);
}

void BlowingControl::stop_pump() {
    blowing_func(false);
}