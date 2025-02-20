#pragma once
#ifndef IO_PHYSICAL_MONITOR_H
#define IO_PHYSICAL_MONITOR_H

#include <Arduino.h>
#include "./io_physical_controller_base.h"

using CHANNEL   = IOPhysicalControllerBase::Channel;
using DIN       = IOPhysicalControllerBase::DigitalInputRole;
using ANIN      = IOPhysicalControllerBase::AnalogInputRole;
using DOUT      = IOPhysicalControllerBase::DigitalOutputRole;
using ANOUT     = IOPhysicalControllerBase::AnalogOutputRole;

class IOPhysicalMonitor : protected IOPhysicalControllerBase
{
private:
    IOPhysicalMonitor()
    {}

    unsigned long _last_heaters_call_ms = 0;
    unsigned long _last_wjacket_call_ms = 0;

    unsigned long _heaters_toggle_idle_ms = 20000;
    unsigned long _wjacket_toggle_idle_ms = 10000;

    bool _heaters_toggle_safe_mode = true;
    bool _wjacket_toggle_safe_mode = false;

    bool _allow_heaters_toggle(bool force = false)
    {
        unsigned long now = millis();

        if (force)
        {
            _last_heaters_call_ms = now;
            return true;
        }

        if (_heaters_toggle_safe_mode)
        {
            if ((now - _last_heaters_call_ms) >= _heaters_toggle_idle_ms)
            {
                _last_heaters_call_ms = now;
                return true;
            }
            else
                return false;
        }

        return true;
    }

    bool _allow_wjacket_toggle(bool force = false)
    {
        unsigned long now = millis();

        if (force)
        {
            _last_wjacket_call_ms = now;
            return true;
        }

        if (_wjacket_toggle_safe_mode)
        {
            if ((now - _last_wjacket_call_ms) >= _wjacket_toggle_idle_ms)
            {
                _last_wjacket_call_ms = now;
                return true;
            }
            else
                return false;
        }

        return true;
    }

public:
    static IOPhysicalMonitor* instance()
    {
        static IOPhysicalMonitor inst;
        return &inst;
    }

    void config_safe_heaters_toggle_mode(bool state) {
        _heaters_toggle_safe_mode = state;
    }

    void config_safe_wjacket_toggle_mode(bool state) {
        _wjacket_toggle_safe_mode = state;
    }

    void config_idle_heaters_toggle_ms(unsigned long _new_idle) {
        _heaters_toggle_idle_ms = _new_idle;
    }

    void config_idle_wjacket_toggle_ms(unsigned long _new_idle) {
        _wjacket_toggle_idle_ms = _new_idle;
    }

    bool get_input_state(DigitalInputRole pin) {
        return this->_digital_input_get_bit(pin);
    }

    float get_temperature_C_product()
    {
        // в private: данного класса реализовать фильтры для температур и аккумулятора
        // логика получения температуры для продукта
        return 45.f;
    }

    float get_temperature_C_wjacket()
    {
        // в private: данного класса реализовать фильтры для температур и аккумулятора
        // логика получения температуры для рубашки
        return 85.f;
    }

    float get_battery_24V_charge()
    {
        // в private: данного класса реализовать фильтры для температур и аккумулятора
        // логика получения заряда аккумулятора
        return 25.3f;
    }

    void set_output_state(DigitalOutputRole pin, bool state, bool force = false)
    {
        switch (pin)
        {
            case DigitalOutputRole::HEATERS_RELAY: {
                if (_allow_heaters_toggle(force))
                    this->change_digital_output_state(state, pin);
            };  break;
                
            case DigitalOutputRole::MIXER_RELAY:
                this->change_digital_output_state(state, pin);
                break;
                
            case DigitalOutputRole::WJACKET_RELAY: {
                if (_allow_wjacket_toggle(force))
                    this->change_digital_output_state(state, pin);
            };  break;
                
            case DigitalOutputRole::WATER_PUMP_RELAY:
                this->change_digital_output_state(state, pin);
                break;
            
            default:
                set_output_states_all(false);
                break;
        }
    }

    void set_output_states_all(bool state) {
        this->change_digital_output_states_all(state);
    }

    void set_motor_speed(short rpm, bool is_fast_mode)
    {
        this->change_analog_output_state(rpm, ANOUT::RPM_SPEED_DRIVER);
        this->change_digital_output_state(rpm > 0, DOUT::MIXER_RELAY);
        this->change_digital_output_state(is_fast_mode, DOUT::MIXER_FAST_MODE_RELAY);
    }
};

#define IOMonitor    IOPhysicalMonitor::instance()

#endif