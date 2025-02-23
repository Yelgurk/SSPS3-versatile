#pragma once
#ifndef IO_PHYSICAL_CONTROLLER_H
#define IO_PHYSICAL_CONTROLLER_H

#include <Arduino.h>
#include <functional>
#include "./io_physical_controller_core.h"
#include "./IOPinMap/Interface/i_pin_map.h"

typedef std::function<bool(unsigned char)>          IOReadDigitalInputHandler;
typedef std::function<void(unsigned char, bool)>    IOWriteDigitalOutputHandler;
typedef std::function<short(unsigned char)>         IOReadAnalogInputHandler;
typedef std::function<void(unsigned char, short)>   IOWriteAnalogOutputHandler;

class IOPhysicalController : public IOPhysicalControllerCore
{
private:
    IOPhysicalController()
    {
        for (unsigned char virtual_pin = DOUT::DIGITAL_OUTPUT_BEGIN; virtual_pin < DOUT::DIGITAL_OUTPUT_END; virtual_pin++)
            this->add_digital_output_handler(
                virtual_pin,
                [](short virtual_pin, bool new_state) { IOPhysicalController::instance()->_digital_write_virtual(virtual_pin, new_state); }
            );

        for (unsigned char virtual_pin = ANOUT::ANALOG_OUTPUT_BEGIN; virtual_pin < ANOUT::ANALOG_OUTPUT_END; virtual_pin++)
            this->add_analog_output_handler(
                virtual_pin,
                [](short virtual_pin, short new_value) { IOPhysicalController::instance()->_analog_write_virtual(virtual_pin, new_value); }
            );
    }

    IOReadDigitalInputHandler   _read_digital_input_handler;
    IOWriteDigitalOutputHandler _write_digital_output_handler;
    IOReadAnalogInputHandler    _read_analog_input_handler;
    IOWriteAnalogOutputHandler  _write_analog_output_handler;

    IPinMap* _pin_map = nullptr;    
    
    bool _digital_read(unsigned char physical_pin)
    {
        if (_read_digital_input_handler)
            return _read_digital_input_handler(physical_pin);

        return false;
    }

    short _analog_read(unsigned char physical_pin)
    {
        if (_read_analog_input_handler)
            return _read_analog_input_handler(physical_pin);

        return (short)-1;
    }

    void _digital_write(unsigned char physical_pin, bool new_state)
    {
        if (_write_digital_output_handler)
            _write_digital_output_handler(physical_pin, new_state);
    }

    void _analog_write(unsigned char physical_pin, short new_value)
    {
        if (_write_analog_output_handler)
            _write_analog_output_handler(physical_pin, new_value);
    }

    void _digital_write_virtual(unsigned char virtual_pin, bool new_state)
    {
        if (_pin_map != nullptr && virtual_pin != -1)
            _digital_write(_pin_map->_vtp_dout(virtual_pin), new_state);
    }

    void _analog_write_virtual(unsigned char virtual_pin, bool new_value)
    {
        if (_pin_map != nullptr && virtual_pin != -1)
            _analog_write(_pin_map->_vtp_anout(virtual_pin), new_value);
    }

    void _update_input_digital_channels()
    {
        if (_pin_map != nullptr)
            for (unsigned char virtual_pin = DIN::DIGITAL_INPUT_BEGIN; virtual_pin < DIN::DIGITAL_INPUT_END; virtual_pin++)
                _digital_io_signal_change_bit(
                    _digital_input_rt,
                    virtual_pin,
                    _digital_read(_pin_map->_vtp_din(virtual_pin))
                );
    }

    void _update_input_analog_channels()
    {
        if (_pin_map != nullptr)
            for (unsigned char virtual_pin = ANIN::ANALOG_INPUT_BEGIN; virtual_pin < ANIN::ANALOG_INPUT_END; virtual_pin++)
                _analog_input_rt[virtual_pin] = _analog_read(_pin_map->_vtp_anin(virtual_pin));
    }

    void _upload_states_into_core()
    {
        set_digital_input_info(this->_digital_input_rt);
        set_digital_output_info(this->_digital_output_rt);
        
        for (unsigned char virtual_pin = ANIN::ANALOG_INPUT_BEGIN; virtual_pin < ANIN::ANALOG_INPUT_END; virtual_pin++)
            set_analog_input_info(virtual_pin, _analog_input_rt[virtual_pin]);

        for (unsigned char virtual_pin = ANOUT::ANALOG_OUTPUT_BEGIN; virtual_pin < ANOUT::ANALOG_OUTPUT_END; virtual_pin++)
            set_analog_output_info(virtual_pin, _analog_output_rt[virtual_pin]);
    }

public:
    static IOPhysicalController* instance()
    {
        static IOPhysicalController inst;
        return &inst;
    }

    void set_handler_read_digital(IOReadDigitalInputHandler handler) {
        _read_digital_input_handler = handler;
    }

    void set_handler_write_digital(IOWriteDigitalOutputHandler handler) {
        _write_digital_output_handler = handler;
    }

    void set_handler_read_analog(IOReadAnalogInputHandler handler) {
        _read_analog_input_handler = handler;
    }

    void set_handler_write_analog(IOWriteAnalogOutputHandler handler) {
        _write_analog_output_handler = handler;
    }

    void set_pin_map(IPinMap* pin_map) {
        this->_pin_map = pin_map;
    }

    void write_digital(DOUT virtual_pin, bool state)
    {
        if (virtual_pin >= DOUT::DIGITAL_OUTPUT_END)
            return;

        _digital_io_signal_change_bit(_digital_output_rt, virtual_pin, state);
    }

    void write_analog(ANOUT virtual_pin, short value)
    {
        if (virtual_pin >= ANOUT::ANALOG_OUTPUT_END)
            return;
        
        _analog_output_rt[virtual_pin] = value;
    }

    void update()
    {
        if (_pin_map == nullptr)
            return;

        _update_input_digital_channels();
        _update_input_analog_channels();
        _upload_states_into_core();
    }
};

#endif // !IO_PHYSICAL_CONTROLLER_H