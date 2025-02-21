#pragma once
#ifndef LIQUID_DISPENSER_HH
#define LIQUID_DISPENSER_HH

#include <Arduino.h>
#include "../ExternalEnvironment/io_physical_monitor.h"

class LiquidDispenser
{
private:
    LiquidDispenser() 
    :   _is_running(false),
        _is_inited(false),
        _initiated_by_pistol(false),
        _initiated_by_button(false),
        _start_delay_done(false),
        _init_time(0),
        _last_update_time(0),
        _idle_start_time(0),
        _start_delay_ms(0),
        _idle_delay_ms(0),
        _target_value(0),
        _remaining_value(0),
        _pump_power_L_per_min(0),
        _flow_rate_ml_per_ms(0.0),
        _is_timer_mode(false),
        _last_btn_state(false),
        _last_pistol_state(false)
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
    void finishDispensing(bool successful)
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
            _is_inited = false;
            _initiated_by_pistol = false;
            _initiated_by_button = false;
            _start_delay_done = false;
            _idle_start_time = 0;
        }
        // Если инициатор не отпущен – оставляем флаги, чтобы не допустить новый запуск,
        // пока пользователь не отпустит курок/кнопку.
    }

    // Состояния процесса
    bool _is_running;
    bool _is_inited;
    bool _initiated_by_pistol;
    bool _initiated_by_button;
    bool _start_delay_done;  // для кнопки: задержка истекла или нет

    // Таймеры (в мс)
    unsigned long _init_time;
    unsigned long _last_update_time;
    unsigned long _idle_start_time;

    // Настраиваемые задержки (в мс)
    unsigned short _start_delay_ms;  // задержка старта для кнопки
    unsigned short _idle_delay_ms;   // задержка ожидания перед завершением, когда инициатор отпущен

    // Параметры раздачи
    unsigned long _target_value;   // заданное количество (в ml или сек)
    unsigned long _remaining_value; // оставшееся количество для раздачи
    unsigned long _pump_power_L_per_min;
    bool _is_timer_mode;           // true, если режим таймера (value в сек), false – режим по мл

    // Скорость потока (рассчитанная из мощности насоса) – мл/мс
    double _flow_rate_ml_per_ms;

    // Для отслеживания переходов состояния инициатора (для контроля, что после завершения пользователь отпустил курок/кнопку)
    bool _last_btn_state;
    bool _last_pistol_state;

public:
    // Геттеры состояний
    bool get_is_running() const {
        return _is_running;
    }

    bool get_is_inited() const {
        return _is_inited;
    }

    // Регистрирует параметры раздачи, если процесс не запущен (и не инициализирован)
    void registerDispense(unsigned long value, unsigned long pump_power_L_per_min, bool is_timer_ss)
    {
        if (!_is_inited && !_is_running)
        {
            _target_value = value;
            _remaining_value = value;
            _pump_power_L_per_min = pump_power_L_per_min;
            _is_timer_mode = is_timer_ss;

            // Перевод мощности (л/мин) в мл/мс: (pump_power * 1000 мл) / 60000 мс (60 сек)
            _flow_rate_ml_per_ms = ((float)pump_power_L_per_min * 1000.f) / 60000.f;
        }
    }

    // Инициализирует процесс раздачи.
    // Если вызов происходит по пистолету (call_by_dispensing_gun==true), задержка старта пропускается.
    // Если по кнопке (call_by_plc_button==true), то будет ожидание задержки (заданной через set_start_delay_after_start_init_by_plc_button)
    void init_disposing(bool call_by_dispensing_gun)
    {
        if (_is_running) return;  // Если процесс уже идёт, не начинаем новый
        _is_inited = true;
        _initiated_by_pistol = call_by_dispensing_gun;
        _initiated_by_button = !call_by_dispensing_gun;
        _start_delay_done = call_by_dispensing_gun;  // для пистолета старт идёт сразу
        _init_time = millis();
        _last_update_time = _init_time;
        _idle_start_time = 0;
    }

    // Задержка старта (в мс) для инициализации по кнопке
    void set_start_delay_after_start_init_by_plc_button(unsigned short delay_ms) {
        _start_delay_ms = delay_ms;
    }

    // Задержка ожидания (в мс) при отпускании инициатора (курка/кнопки, которая запустила процесс раздачи)
    void set_idle_before_reset_when_is_running(unsigned short delay_ms) {
        _idle_delay_ms = delay_ms;
    }

    // Основной метод controller(), выполняющий расчёты раздачи на основе разницы времени между вызовами (dt)
    // Параметр is_btn_pressed актуален для кнопочного режима. Если инициатор отпущен – запускается idle-тimer.
    void controller(bool is_btn_pressed = false)
    {
        if (!_is_inited) return;

        unsigned long current_time = millis();
        unsigned long dt = current_time - _last_update_time;
        _last_update_time = current_time;

        // Если инициатор раздачи по кнопке и задержка старта не истекла, проверяем её:
        if (_initiated_by_button && !_start_delay_done) {
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
            _idle_start_time = 0;
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
                finishDispensing(true); // true означает успешное завершение
            }
        } else {  // Инициатор отпущен
            if (_is_running) {
                // Если инициатор отпущен, запускаем idle-таймер
                if (_idle_start_time == 0) {
                    _idle_start_time = current_time;
                    _stop_pump();
                }
                if (current_time - _idle_start_time >= _idle_delay_ms) {
                    finishDispensing(false);
                }
            }
        }
    }

    // Перегруженный метод update для работы с кнопкой.
    // Здесь реализована проверка: если в предыдущем цикле кнопка не была отпущена после завершения,
    // новый процесс не запускается.
    void update(bool is_btn_pressed) {
        // Если нет активного процесса, то начинаем новый только при переходе с "не нажата" в "нажата"
        if (!_is_inited && !_is_running) {
            if (is_btn_pressed && !_last_btn_state) {  // rising edge
                init_disposing(false, true);
            }
        }
        // Если процесс завершён успешно, требуем отпускания инициатора для нового запуска
        if (!_is_inited && !_is_running) {
            if (is_btn_pressed) {
                // Инициатор всё ещё удерживается – не начинаем новый процесс
                _last_btn_state = is_btn_pressed;
                return;
            }
        }
        controller(is_btn_pressed);
        _last_btn_state = is_btn_pressed;
    }

    // Перегруженный метод update для работы с пистолетным триггером.
    // Аналогично, новый процесс начинается только при обнаружении перехода из отпущенного состояния.
    void update()
    {
        bool current_pistol = IOMonitor::instance().isPistolTriggerActive();
        if (!_is_inited && !_is_running) {
            if (current_pistol && !_last_pistol_state) { // rising edge
                init_disposing(true, false);
            }
        }
        if (!_is_inited && !_is_running) {
            if (current_pistol) {  // если курок всё ещё не отпущен, не начинаем новый процесс
                _last_pistol_state = current_pistol;
                return;
            }
        }
        
        controller();
        _last_pistol_state = current_pistol;
    }
};


#endif // !LIQUID_DISPENSER_HH