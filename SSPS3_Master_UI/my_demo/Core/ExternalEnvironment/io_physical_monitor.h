#pragma once
#ifndef IO_PHYSICAL_MONITOR_H
#define IO_PHYSICAL_MONITOR_H

#include <Arduino.h>
#include "./io_physical_controller_core.h"
#include "./IOMonitorFilters/filters.h"

class IOPhysicalMonitor : public IOPhysicalControllerCore
{
private:
    IOPhysicalMonitor()
    {
        _f_temp_C_product.add_converter([](short value) {
            return (float)(value - 1000) / 2.5f;
        });

        _f_temp_C_wJacket.add_converter([](short value) {
            return (float)(value - 1000) / 2.5f;
        });

        _f_value_24V_batt.add_converter([](short value) {
            return (float)(value - 1000) / 6.0f;
        });
    }

    IValueTFilter<short, float>& _f_temp_C_product = FilterExponentialSmoothing<short, float>(0.06f).get_i_value_filter(); 
    IValueTFilter<short, float>& _f_temp_C_wJacket = FilterExponentialSmoothing<short, float>(0.06f).get_i_value_filter();
    IValueTFilter<short, float>& _f_value_24V_batt = FilterExponentialSmoothing<short, float>(0.03f).get_i_value_filter();

    unsigned long _last_temp_C_product_call_ms = 0;     // Добавлено
    unsigned long _last_temp_C_wJacket_call_ms = 0;     // Добавлено
    unsigned long _last_value_24v_batt_call_ms = 0;     // Добавлено
    unsigned long _temp_C_product_call_delay_ms = 500;  // Добавлено    
    unsigned long _temp_C_wJacket_call_delay_ms = 500;  // Добавлено    
    unsigned long _value_24v_batt_call_delay_ms = 1000; // Добавлено    

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

    bool get_input_state(DIN pin) {
        return this->_digital_input_get_bit(pin);
    }

    float get_temperature_C_product()
    {
        // Добавлено ---> if ({таймер выполнен})
        // Добавлено ---> {
        // Добавлено --->     {сохраняем новое кол-во мс}
        // Добавлено --->     _f_temp_C_product.add_value(this->_analog_input_get_value(ANIN::TEMP_C_PRODUCT_SENS));
        // Добавлено ---> }
        // Добавлено ---> 
        // Добавлено ---> return _f_temp_C_product.get_value_converted();
    }

    float get_temperature_C_wjacket()
    {
        // Добавлено ---> if ({таймер выполнен})
        // Добавлено ---> {
        // Добавлено --->     {сохраняем новое кол-во мс}
        // Добавлено --->     _f_temp_C_wJacket.add_value(this->_analog_input_get_value(ANIN::TEMP_C_WJACKET_SENS));
        // Добавлено ---> }
        // Добавлено ---> 
        // Добавлено ---> return _f_temp_C_wJacket.get_value_converted();
    }

    float get_battery_24V_charge()
    {
        // Добавлено ---> if ({таймер выполнен})
        // Добавлено ---> {
        // Добавлено --->     {сохраняем новое кол-во мс}
        // Добавлено --->     _f_value_24V_batt.add_value(this->_analog_input_get_value(ANIN::V24_DC_BATT_SENS));
        // Добавлено ---> }
        // Добавлено ---> 
        // Добавлено ---> return _f_value_24V_batt.get_value_converted();
    }

    void set_output_state(DOUT pin, bool state, bool force = false)
    {
        switch (pin)
        {
            case DOUT::HEATERS_RELAY: {
                if (_allow_heaters_toggle(force))
                    this->change_digital_output_pin_state(pin, state);
            };  break;
                
            case DOUT::MIXER_RELAY:
                this->change_digital_output_pin_state(pin, state);
                break;
                
            case DOUT::WJACKET_RELAY: {
                if (_allow_wjacket_toggle(force))
                    this->change_digital_output_pin_state(pin, state);
            };  break;
                
            case DOUT::WATER_PUMP_RELAY:
                this->change_digital_output_pin_state(pin, state);
                break;

            case DOUT::MIXER_FAST_MODE_RELAY:
                this->change_digital_output_pin_state(pin, state);
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
        this->set_analog_output_info(ANOUT::RPM_SPEED_DRIVER, rpm);
        this->change_digital_output_pin_state(DOUT::MIXER_RELAY, rpm > 0);
        this->change_digital_output_pin_state(DOUT::MIXER_FAST_MODE_RELAY, is_fast_mode);
    }
};

#define IOMonitor    IOPhysicalMonitor::instance()

#endif