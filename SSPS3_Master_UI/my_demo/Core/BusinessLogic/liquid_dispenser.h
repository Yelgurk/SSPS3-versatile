#pragma once
#ifndef LIQUID_DISPENSER_HH
#define LIQUID_DISPENSER_HH

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

        if (successful)
        {
            // Делаем что-то, если ок
        }
        else
        {
            // делаем что-то, если не ок
        }

        // Сбрасываем флаги только если инициатор отпущен
        bool initiator_released = false;
        if (_initiated_by_button)
        {
            initiator_released = !_last_btn_state;
        }
        else
        if (_initiated_by_pistol)
        {
            initiator_released = !_get_dispenser_trigger_state();
        }

        if (initiator_released)
        {
            _is_running = false;
            _initiated_by_pistol = false;
            _initiated_by_button = false;
        }
        // Если инициатор не отпущен – оставляем флаги, чтобы не допустить новый запуск,
        // пока пользователь не отпустит курок/кнопку.
    }

    // Настраиваемые задержки (в мс)
    unsigned short _start_delay_ms          = 2000; // задержка перед стартом, если инициатор раздачи - кнопка
    unsigned short _idle_delay_ms           = 0;    // задержка ожидания перед завершением, когда инициатор отпущен

    // Состояния процесса
    bool _is_running            = false;
    bool _initiated_by_pistol   = false;
    bool _initiated_by_button   = false;
    bool _initiated_by_both     = false;
    // _initiated_by_both: нужно для случаев, когда курок на пистолете раздатчсике поломался, его заклинило
    // в таком случае мы берём за модель поведения _initiated_by_button, игнорируя _initiated_by_pistol

    // Таймеры (в мс)
    unsigned long _timer_init_time          = 0;
    unsigned long _timer_last_update_time   = 0;
    unsigned long _timer_idle_on_start_time = 0;

    // Зарегистрированые переменные (независимо от состояния работы)
    float _registered_value                 = 0;
    float _registered_flow_rate_ml_per_ss   = 0;
    bool _registered_is_timer_mode          = false;         

    // Параметры раздачи
    float _target_value                 = 0;        // заданное количество (в ml или сек)
    float _target_remaining_value       = 0;        // оставшееся количество для раздачи
    float _target_flow_rate_ml_per_ms   = 0;
    bool _target_is_timer_mode          = false;    // true, если режим таймера (value в сек), false – режим по мл

    // Для отслеживания переходов состояния инициатора (для контроля, что после завершения пользователь отпустил курок/кнопку)
    bool _on_start_btn_state    = false;
    bool _on_start_pistol_state = false;
    bool _actual_btn_state      = false;
public:
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
    }

    // Инициализирует процесс раздачи.
    // Если вызов происходит по пистолету (call_by_dispensing_gun==true), задержка старта пропускается.
    // Если по кнопке (call_by_plc_button==true), то будет ожидание задержки (заданной через set_start_delay_after_start_init_by_plc_button)
    void init_disposing()
    {
        if (_is_running)
            return;  // Если процесс уже идёт, не начинаем новый

        _target_value               = _registered_value;           
        _target_remaining_value     = _registered_value;
        _target_flow_rate_ml_per_ms = _registered_flow_rate_ml_per_ss;
        _target_is_timer_mode       = _registered_is_timer_mode;

        _initiated_by_pistol        = _get_dispenser_trigger_state();
        _initiated_by_button        = _actual_btn_state;
        _initiated_by_both          = _initiated_by_pistol && _initiated_by_button;
        
        // без _is_init тут будет постоянно переназначаться таймер старта, тем самым старт по кнопке
        // от ПЛК вечно будет "гнаться" в controller() и раздача не начнётся. Решить эту пробелму добавением _is_init
        // и как-то обыграть его в controller() на этапе _timer_idle_on_start_time проверки пока не отпущена кнопка (в таком сучае 
        // все три переменные не должны менять своё состояние)
        _timer_init_time            = millis();
        _timer_last_update_time     = _timer_init_time;
        _timer_idle_on_start_time   = (_initiated_by_pistol && !_initiated_by_both) ? 0 : _start_delay_ms;
    }

    // Задержка старта (в мс) для инициализации по кнопке
    void set_delay_before_start_on_btn_init(unsigned short delay_ms) {
        _start_delay_ms = delay_ms;
    }

    // Задержка ожидания (в мс) при отпускании инициатора (курка/кнопки, которая запустила процесс раздачи)
    void set_idle_before_reset_when_is_running(unsigned short delay_ms) {
        _idle_delay_ms = delay_ms;
    }

    void update()
    {
        init_disposing();

        // тут логика по проверке, отпущены ли были кнопки через _on_start_btn_state и _on_start_pistol_state

    }

    // Основной метод controller(), выполняющий расчёты раздачи на основе разницы времени между вызовами (dt)
    // Параметр is_btn_pressed актуален для кнопочного режима. Если инициатор отпущен – запускается idle-тimer.
    void controller()
    {
        init_disposing();
        if (!_is_inited) return;

        unsigned long current_time = millis();
        unsigned long dt = current_time - _last_update_time;
        _last_update_time = current_time;

        // Если инициатор раздачи по кнопке и задержка старта не истекла, проверяем её:
        if (_initiated_by_button) {
            if (is_btn_pressed && (current_time - _init_time >= _start_delay_ms))
                _start_delay_done = true;
            else
                return;  // Ждём, пока кнопка удерживается и задержка истекает
        }

        // Определяем, активен ли инициатор (курок или кнопка)
        bool initiator_active = false;
        if (_initiated_by_pistol) {
            initiator_active = IOMonitor::instance().isPistolTriggerActive();
        } else if (_initiated_by_button) {
            initiator_active = is_btn_pressed;
        }

        if (initiator_active) {
            // Если инициатор активно удерживается, сбрасываем idle-таймер
            _idle_before_start_time = 0;
            if (!_is_running) {
                _is_running = true;
                _start_pump();
            }
            // Расчёт выданного объёма за dt (в мл или сек, в зависимости от режима)
            unsigned long dispensed = static_cast<unsigned long>(_flow_rate_ml_per_ms * dt);
            if (dispensed > _remaining_value)
                dispensed = _remaining_value;
            _remaining_value -= dispensed;

            Serial.print("Dispensed: ");
            Serial.print(_target_value - _remaining_value);
            Serial.print(_is_timer_mode ? " sec" : " ml");
            Serial.print(" / ");
            Serial.println(_target_value);

            // Если требуемое количество выдано – завершаем процесс.
            // Важно: если инициатор всё ещё удерживается, не сбрасываем флаги, чтобы не допустить нового запуска.
            if (_remaining_value == 0) {
                _finish_dispensing(true); // true означает успешное завершение
            }
        } else {  // Инициатор отпущен
            if (_is_running) {
                // Если инициатор отпущен, запускаем idle-таймер
                if (_idle_before_start_time == 0) {
                    _idle_before_start_time = current_time;
                    _stop_pump();
                }
                if (current_time - _idle_before_start_time >= _idle_delay_ms) {
                    _finish_dispensing(false);
                }
            }
        }
    }
};


#endif // !LIQUID_DISPENSER_HH