#pragma once
#ifndef LIQUID_DISPENSER_H
#define LIQUID_DISPENSER_H

#include <Arduino.h>
#include "../ExternalEnvironment/io_physical_monitor.h"

class LiquidDispenser
{
private:
    LiquidDispenser() 
    {}

    // Включает насос через IOMonitor
    void _start_pump() {
        IOMonitor->set_output_state(DOUT::WATER_PUMP_RELAY, true);
    }
    // Выключает насос через IOMonitor
    void _stop_pump() {
        IOMonitor->set_output_state(DOUT::WATER_PUMP_RELAY, false);
    }

    bool _get_dispenser_trigger_state() {
        return IOMonitor->get_input_state(DIN::DISPENSER_SIGNAL);
    }

    // Завершает процесс раздачи.
    // Если successful==true, значит задача завершена успешно, и при этом мы проверяем, отпущен ли инициатор.
    // Если инициатор всё ещё удерживается, флаги не сбрасываются, чтобы не допустить повторного запуска.
    void _finish_dispensing(bool successful)
    {
        _stop_pump();
        _is_inited = false;
        _is_running = false;

        if (successful)
        {
            // Делаем что-то, если ок
        }
        else
        {
            // делаем что-то, если не ок
        }
    }

    // Настраиваемые задержки (в мс)
    unsigned long _start_delay_ms           = 2000; // задержка перед стартом, если инициатор раздачи - кнопка
    unsigned long _idle_delay_ms            = 0;    // задержка ожидания перед завершением, когда инициатор отпущен
    bool _start_delay_done                  = false;

    // Состояния процесса
    bool _is_inited             = false;
    bool _is_running            = false;
    
    // Таймеры (в мс)
    unsigned long _timer_init_time                  = 0;
    unsigned long _timer_last_litres_update_time    = 0;
    unsigned long _timer_last_stopwatch_update_time = 0;
    unsigned long _timer_idle_on_start_time         = 0;

    // Зарегистрированые переменные (независимо от состояния работы)
    float _registered_value                 = 0;
    float _registered_flow_rate_ml_per_ss   = 0;
    bool _registered_is_timer_mode          = false;         

    // Параметры раздачи
    float _target_value                 = 0;        // заданное количество (в ml или сек)
    float _target_remaining_value       = 0;        // оставшееся количество для раздачи
    float _target_flow_rate_ml_per_ms   = 0;
    bool _target_is_timer_mode          = false;    // true, если режим таймера (value в сек), false – режим по мл

    // Для отслеживания переходов состояния инициатора-кнопки
    // _initiated_by_both: нужно для случаев, когда курок на пистолете раздатчсике поломался, его заклинило
    // в таком случае мы берём за модель поведения _initiated_by_button, игнорируя _initiated_by_pistol
    bool _actual_btn_state      = false;
    bool _initiated_by_pistol   = false;
    bool _initiated_by_button   = false;
    bool _initiated_by_both     = false;

public:
    static LiquidDispenser* instance()
    {
        static LiquidDispenser inst;
        return &inst;
    }

    // Геттеры состояний
    bool get_is_running() const {
        return _is_running;
    }

    void set_plc_btn_state(bool is_btn_pressed) {
        _actual_btn_state = is_btn_pressed;
    }

    // Регистрирует параметры раздачи, если процесс не запущен (и не инициализирован)
    void register_dispense_choise(unsigned long value, float pump_power_L_per_min, bool is_timer_ss)
    {
        _registered_value               = value;
        _registered_flow_rate_ml_per_ss = (pump_power_L_per_min * 1000.f) / 60000.f;
        _registered_is_timer_mode       = is_timer_ss;
        
        // Если проинициализировано, но не начата работа насоса
        // значит мы запускаемся по кнопке и отрабатывает таймер
        // _start_delay_ms в update(). В таком случае, пока идёт
        // выдержка старта, можем изменить раздаваемую дозу через ре-инициализацию
        // на что и указывает флаг true для init_disposing(...)
        if (!_is_running && _is_inited)
            init_disposing(true);
    }

    // Инициализирует процесс раздачи.
    // Если вызов происходит по пистолету (call_by_dispensing_gun==true), задержка старта пропускается.
    // Если по кнопке (call_by_plc_button==true), то будет ожидание задержки (заданной через set_start_delay_after_start_init_by_plc_button)
    void init_disposing(bool reinit = false)
    {
        if (!reinit && (_is_running || _is_inited))
            return;

        _target_value               = _registered_value;           
        _target_remaining_value     = _registered_value;
        _target_flow_rate_ml_per_ms = _registered_flow_rate_ml_per_ss;
        _target_is_timer_mode       = _registered_is_timer_mode;

        if (!reinit && !_is_running)
        {
            _initiated_by_pistol    = _get_dispenser_trigger_state();
            _initiated_by_button    = _actual_btn_state;
            _initiated_by_both      = _initiated_by_pistol && _initiated_by_button;
            _start_delay_done       = false;
        }

        _timer_init_time                    = millis();
        _timer_last_litres_update_time      = _timer_init_time;
        _timer_last_stopwatch_update_time   = _timer_init_time;
        _timer_idle_on_start_time           = (_initiated_by_pistol && !_initiated_by_both) ? 0 : _start_delay_ms;
    
        _is_inited = true;
    }

    // Задержка старта (в мс) для инициализации по кнопке
    void set_delay_before_start_on_btn_init(unsigned short delay_ms) {
        _start_delay_ms = delay_ms;
    }

    // Задержка ожидания (в мс) при отпускании инициатора (курка/кнопки, которая запустила процесс раздачи)
    void set_idle_before_reset_when_is_running(unsigned short delay_ms) {
        _idle_delay_ms = delay_ms;
    }

    // Основной метод controller(), выполняющий расчёты раздачи на основе разницы времени между вызовами (dt)
    // Параметр is_btn_pressed актуален для кнопочного режима. Если инициатор отпущен – запускается idle-тimer.
    void update()
    {
        // Если процесс еще не инициирован, пробуем инициализировать, если один из инициаторов нажат
        if (!_is_running)
        {
            if (!_is_inited)
            {
                // Проверка на то, "отпущены" ли инициализаторы старта раздачи
                if (_initiated_by_button && !_actual_btn_state)
                    _initiated_by_button = false;

                if (_initiated_by_pistol && _get_dispenser_trigger_state())
                    _initiated_by_pistol = false;

                if (_initiated_by_both)
                    _initiated_by_both = _initiated_by_button || _initiated_by_pistol;

                // Если оба отпущены и один из стал активен, то стартуем 
                if (!_initiated_by_both && !_initiated_by_button && !_initiated_by_pistol)
                {
                    if (_actual_btn_state || _get_dispenser_trigger_state())
                        init_disposing();
                }
                else
                if (_initiated_by_both && !_initiated_by_button)
                {
                    // Если флаг "оба удержаны", не глядя на уже завершённую раздачу,
                    // значит "залип" курок раздачи на пистолете, тогда учитываем только
                    // кнопку на контроллере
                    if (_actual_btn_state)
                        init_disposing();
                }
            }

            return; // Ничего не делаем, если инициаторы не активны
        }

        // Для кнопочного режима: ожидаем, пока задержка старта истечет
        if (_initiated_by_button && !_start_delay_done)
        {
            if (_actual_btn_state && (millis() - _timer_init_time >= _start_delay_ms))
            {
                _start_delay_done                   = true;
                _timer_last_litres_update_time      = millis();
                _timer_last_stopwatch_update_time   = _timer_last_litres_update_time;
            }
            else
            if (!_actual_btn_state)
            {
                _finish_dispensing(false);
                return;
            }
        }

        _is_running = true;

        // Определяем активность инициатора: для пистолета – через IOMonitor, для кнопки – _actual_btn_state
        bool initiator_active = false;
        if (_initiated_by_pistol)
            initiator_active = _get_dispenser_trigger_state();
        else
        if (_initiated_by_button)
            initiator_active = _actual_btn_state;

        unsigned long current_time = millis();

        if (initiator_active)
        {
            // Если инициатор удерживается, сбрасываем idle-таймер
            _timer_idle_on_start_time = 0;
            _start_pump();

            if (_target_is_timer_mode)
            {
                // Реализация таймера, который срабатывает раз в секунду
                if (current_time - _timer_last_stopwatch_update_time >= 1000)
                {
                    _timer_last_stopwatch_update_time = current_time;
                    _target_remaining_value--;
                }
            }
            else
            {
                unsigned long ms_gone = current_time - _timer_last_litres_update_time;
                _timer_last_litres_update_time = current_time;

                // Вычисляем выданное количество за интервал dt
                float dispensed = _target_flow_rate_ml_per_ms * ms_gone;

                if (dispensed > _target_remaining_value)
                    dispensed = _target_remaining_value;

                _target_remaining_value -= dispensed;
            }

            if (_target_remaining_value <= 0.f)
                _finish_dispensing(true);
        }
        else
        {
            _stop_pump();

            if (_timer_idle_on_start_time == 0)
                _timer_idle_on_start_time = current_time;

            if (current_time - _timer_idle_on_start_time >= _idle_delay_ms)
                _finish_dispensing(false);
        }
    }
};


#endif // !LIQUID_DISPENSER_H