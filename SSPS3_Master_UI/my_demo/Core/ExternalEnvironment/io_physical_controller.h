#pragma once
#ifndef IO_PHYSICAL_CONTROLLER_H
#define IO_PHYSICAL_CONTROLLER_H

#include <Arduino.h>
#include <functional>
#include "./io_physical_controller_core.h"
#include "./IOControllerPinMap/i_pin_map.h"

typedef std::function<bool(DIN)>            IOReadDigitalInputHandler;
typedef std::function<void(DOUT, bool)>     IOWriteDigitalOutputHandler;
typedef std::function<short(ANIN)>          IOReadAnalogInputHandler;
typedef std::function<void(ANOUT, short)>   IOWriteAnalogOutputHandler;

class IOPhysicalController
{
private:
    IOPhysicalController() {
        IOPhysicalControllerCore::instance();
    }

    IOReadDigitalInputHandler   _read_digital_input_handler;
    IOWriteDigitalOutputHandler _write_digital_output_handler;
    IOReadAnalogInputHandler    _read_analog_input_handler;
    IOWriteAnalogOutputHandler  _write_analog_output_handler;

    bool _digital_read(DIN pin)
    {
        if (_read_digital_input_handler && pin < DIN::DIGITAL_INPUT_END)
            return _read_digital_input_handler(pin);

        return false;
    }

    void _digital_write(DOUT pin, bool new_state)
    {
        if (_write_digital_output_handler && pin < DOUT::DIGITAL_OUTPUT_END)
            _write_digital_output_handler(pin, new_state);
    }

    short _analog_read(ANIN pin)
    {
        if (_read_analog_input_handler && pin < ANIN::ANALOG_INPUT_END)
            return _read_analog_input_handler(pin);

        return (short)-1;
    }

    void _analog_write(ANOUT pin, short value)
    {
        if (_write_analog_output_handler && pin < ANOUT::ANALOG_OUTPUT_END)
            _write_analog_output_handler(pin, value);
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

    // реализовать логику работы с физическими пинами через интерфейс IPinMap (i_pin_map.h)
    // относительно "виртуальных" пинов, а именно:
    // - enum::DigitalInputRole     (DIN)
    // - enum::DigitalOutputRole    (DOUT)
    // - enum::AnalogInputRole      (ANIN)        
    // - enum::AnalogOutputRole     (ANOUT)
    // работая с их (виртуальные пины) состояниями в unsigned char хранилищах стейтов, а именно:
    // - unsigned char _digital_input           
    // - unsigned char _digital_output          
    // - short _analog_input[...]  
    // - short _analog_output[...]
    //
};

#endif // !IO_PHYSICAL_CONTROLLER_H