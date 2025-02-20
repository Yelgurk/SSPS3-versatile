#pragma once
#ifndef IO_PHYSICAL_CONTROLLER_H
#define IO_PHYSICAL_CONTROLLER_H

#include "./io_physical_controller_base.h"

using CHANNEL   = IOPhysicalControllerBase::Channel;
using DIN       = IOPhysicalControllerBase::DigitalInputRole;
using ANIN      = IOPhysicalControllerBase::AnalogInputRole;
using DOUT      = IOPhysicalControllerBase::DigitalOutputRole;
using ANOUT     = IOPhysicalControllerBase::AnalogOutputRole;

class IOPhysicalController : protected IOPhysicalControllerBase
{
private:
    IOPhysicalController()
    {}

    unsigned long _last_heaters_call_ms = 0;
    unsigned long _last_wjacket_call_ms = 0;

    unsigned long _idle_heaters_toggle_ms = 20000;
    unsigned long _idle_wjacket_toggle_ms = 10000;

    bool _safe_heaters_toggle_mode = true;
    bool _safe_wjacket_toggle_mode = false;

    bool _allow_heaters_toggle(bool force = false)
    {
        // если force то обновить таймер последнего вызова и вернуть true

        // если _safe_heaters_toggle_mode == true
        // то возвращать false пока таймер не преодолеет задержку _idle_heaters_toggle_ms
        // возвращает true/false

        // если _safe_heaters_toggle_mode == false
        // то посто вернуть true
    }

    bool _allow_wjacket_toggle(bool force = false)
    {
        // если force то обновить таймер последнего вызова и вернуть true

        // если _safe_wjacket_toggle_mode == true
        // то возвращать false пока таймер не преодолеет задержку _idle_wjacket_toggle_ms
        // возвращает true/false

        // если _safe_wjacket_toggle_mode == false
        // то посто вернуть true
    }

public:
    static IOPhysicalController* instance()
    {
        static IOPhysicalController inst;
        return &inst;
    }

    void set_safe_heaters_toggle_mode(bool state) {
        _safe_heaters_toggle_mode = state;
    }

    void set_safe_wjacket_toggle_mode(bool state) {
        _safe_wjacket_toggle_mode = state;
    }

    bool get_input_state(DigitalInputRole pin) {
        return this->_digital_input_get_bit(pin);
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
};

#define IOController    IOPhysicalController::instance()

#endif