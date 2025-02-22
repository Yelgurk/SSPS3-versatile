#pragma once
#ifndef IO_PHYSICAL_CONTROLLER_BASE_H
#define IO_PHYSICAL_CONTROLLER_BASE_H

#include <vector>
#include <functional>

typedef std::function<void(bool)>   IODigitalSignalHandler;
typedef std::function<void(short)>  IOAnalogSignalHandler;

class IOPhysicalControllerBase
{
public:
    enum Channel : unsigned char
    {
        DIGITAL_INPUT   = 0b00010000,
        ANALOG_INPUT    = 0b00100000,
        DIGITAL_OUTPUT  = 0b01000000,
        ANALOG_OUTPUT   = 0b10000000
    };

    enum DigitalInputRole : unsigned char
    {
        DIGITAL_INPUT_BEGIN = 0,
        DISPENSER_SIGNAL    = 0,
        V380_SIGNAL,
        MIXER_ERROR_SIGNAL,
        STOP_BTN_SIGNAL,
        WATER_JACKET_SIGNAL,
        DIGITAL_INPUT_END,
        DIGITAL_INPUT_LIMIT = 0b00001111
    };

    enum DigitalOutputRole : unsigned char
    {
        DIGITAL_OUTPUT_BEGIN    = 0,
        HEATERS_RELAY           = 0,
        MIXER_RELAY,
        WJACKET_RELAY,
        WATER_PUMP_RELAY,
        MIXER_FAST_MODE_RELAY,
        DIGITAL_OUTPUT_END,
        DIGITAL_OUTPUT_LIMIT    = 0b00001111
    };

    enum AnalogInputRole : unsigned char
    {
        ANALOG_INPUT_BEGIN  = 0,
        TEMP_C_PROFUCT_SENS = 0,
        TEMP_C_WJACKET_SENS,
        v24_DC_BATT_SENS,
        ANALOG_INPUT_END,
        ANALOG_INPUT_LIMIT  = 0b00001111
    };

    enum AnalogOutputRole : unsigned char
    {
        ANALOG_OUTPUT_BEGIN = 0,
        RPM_SPEED_DRIVER    = 0,
        ANALOG_OUTPUT_END,
        ANALOG_OUTPUT_LIMIT = 0b00001111
    };

private:
    IOPhysicalControllerBase()
    {}

    unsigned char _digital_input            = 0b00000000;   // доступно DIGITAL_INPUT_END цифровых входов, но не более DIGITAL_INPUT_LIMIT. Каждый бит как элемент массива согласно размеру типа переменной-хранилища
    unsigned char _digital_output           = 0b00000000;   // доступно DIGITAL_OUTPUT_END цифровых выходов, но не более DIGITAL_OUTPUT_LIMIT. Каждый бит как элемент массива согласно размеру типа переменной-хранилища
    short _analog_input[ANALOG_INPUT_END]   = { 0 };        // доступно ANALOG_INPUT_END аналоговых 12 бит (16 бит) входов, но не более ANALOG_INPUT_LIMIT.
    short _analog_output[ANALOG_OUTPUT_END] = { 0 };        // доступен ANALOG_OUTPUT_END аналоговый 12 бит (16 бит) выходов, но не более ANALOG_OUTPUT_LIMIT.

    std::vector<std::pair<unsigned char, IODigitalSignalHandler>> _digital_signal_handlers;
    std::vector<std::pair<unsigned char, IOAnalogSignalHandler>> _analog_signal_handlers;

    unsigned char const _channel_mask   = 0b11110000;
    unsigned char const _pin_index_mask = 0b00001111;

    void _digital_io_state_handler(unsigned char& old_state, unsigned char& new_state, unsigned char begin_index, unsigned char end_index, Channel channel)
    {
        for (unsigned char pin = begin_index; pin < end_index; pin++)
        {
            bool _is_equal = _digital_io_signals_comparator(old_state, new_state, pin); 

            if (!_is_equal)
            {
                unsigned char _instr = _formulate_instructions(channel, pin);
                bool _new_state = _digital_io_signal_get_bit(new_state, pin);

                for (auto &handler_pair : _digital_signal_handlers)
                {
                    if (handler_pair.first == _instr)
                        handler_pair.second(_new_state);
                }
            }   
        }
    }

    void _analog_io_value_handler(short& old_value, short& new_value, unsigned char pin, Channel channel)
    {
        if (old_value != new_value)
        {
            unsigned char _instr = _formulate_instructions(channel, pin);

            for (auto &handler_pair : _analog_signal_handlers)
            {
                if (handler_pair.first == _instr)
                    handler_pair.second(new_value);
            }
        }
    }

public:
    static IOPhysicalControllerBase* instance()
    {
        static IOPhysicalControllerBase inst;
        return &inst;
    }

    //------------------------------------------------------------------------
    // Методы с префиксом _ нежелательны для использования пользователем извне 
    //------------------------------------------------------------------------

    bool _digital_io_signals_comparator(unsigned char _old_states_container, unsigned char _new_states_container, unsigned char _index)
    {
        bool oldBit = ((_old_states_container & (1 << _index)) != 0);
        bool newBit = ((_new_states_container & (1 << _index)) != 0);
        return oldBit == newBit;
    }

    bool _digital_io_signal_get_bit(unsigned char _bit_family, unsigned char _index) {
        return (_bit_family & (1 << _index)) != 0;
    }

    bool _digital_input_get_bit(unsigned char _index) {
        return _digital_io_signal_get_bit(_digital_input, _index);
    }

    void _digital_io_signal_change_bit(unsigned char& _bit_family, unsigned char _index, bool _new_state)
    {
        if (_new_state)
            _bit_family |= (1 << _index);
        else
            _bit_family &= ~(1 << _index);
    }

    unsigned char _formulate_instructions(Channel _channel_enum, unsigned char _pin) {
        return (0b00000000 | _channel_enum | _pin);
    }

    // 2 extra метода, на всякий случай, для извлечения номера пина или самого канала
    unsigned char _get_pin_index_from_instruction(unsigned char instruction) {
        return (instruction & _pin_index_mask);
    }

    Channel _get_channel_from_instruction(unsigned char instruction) {
        return static_cast<Channel>(instruction & _channel_mask);
    }

    //----------------------------------------------------------
    // Методы без префикса предназначены для использования извне
    //----------------------------------------------------------

    void set_digital_input_info(unsigned char digital_input_received)
    {
        _digital_io_state_handler(
            this->_digital_input,
            digital_input_received,
            DIGITAL_INPUT_BEGIN,
            DIGITAL_INPUT_END,
            DIGITAL_INPUT
        );

        this->_digital_input = digital_input_received;
    }

    void set_digital_output_info(unsigned char digital_output_received)
    {
        _digital_io_state_handler(
            this->_digital_output,
            digital_output_received,
            DIGITAL_OUTPUT_BEGIN,
            DIGITAL_OUTPUT_END,
            DIGITAL_OUTPUT
        );

        this->_digital_output = digital_output_received;
    }

    void set_analog_input_info(short analog_input_received, unsigned char pin)
    {
        if (pin >= ANALOG_INPUT_END)
            return;

        _analog_io_value_handler(
            _analog_input[pin],
            analog_input_received,
            pin,
            ANALOG_INPUT
        );

        _analog_input[pin] = analog_input_received;
    }

    void set_analog_output_info(short analog_output_received, unsigned char pin)
    {
        if (pin >= ANALOG_OUTPUT_END)
            return;

        _analog_io_value_handler(
            _analog_output[pin],
            analog_output_received,
            pin,
            ANALOG_OUTPUT
        );

        _analog_output[pin] = analog_output_received;
    }

    void change_digital_output_pin_state(bool new_state, unsigned char pin)
    {
        if (pin >= DIGITAL_OUTPUT_END)
            return;

        unsigned char _digital_output_new = this->_digital_output;
        _digital_io_signal_change_bit(_digital_output_new, pin, new_state);

        _digital_io_state_handler(
            this->_digital_output,
            _digital_output_new,
            DIGITAL_OUTPUT_BEGIN,
            DIGITAL_OUTPUT_END,
            DIGITAL_OUTPUT
        );

        this->_digital_output = _digital_output_new;
    }

    void change_digital_output_states_all(bool new_state) {
        set_digital_output_info(new_state ? 0xFF : 0x00);
    }

    void add_digital_input_handler(DigitalInputRole pin, IODigitalSignalHandler handler) {
        _digital_signal_handlers.push_back({ _formulate_instructions(Channel::DIGITAL_INPUT, pin), handler});
    }

    void add_digital_output_handler(DigitalOutputRole pin, IODigitalSignalHandler handler) {
        _digital_signal_handlers.push_back({ _formulate_instructions(Channel::DIGITAL_OUTPUT, pin), handler});
    }

    void add_analog_input_handler(AnalogInputRole pin, IOAnalogSignalHandler handler) {
        _analog_signal_handlers.push_back({ _formulate_instructions(Channel::ANALOG_INPUT, pin), handler});
    }

    void add_analog_output_handler(AnalogOutputRole pin, IOAnalogSignalHandler handler) {
        _analog_signal_handlers.push_back({ _formulate_instructions(Channel::ANALOG_OUTPUT, pin), handler});
    }
};

using CHANNEL   = IOPhysicalControllerBase::Channel;
using DIN       = IOPhysicalControllerBase::DigitalInputRole;
using ANIN      = IOPhysicalControllerBase::AnalogInputRole;
using DOUT      = IOPhysicalControllerBase::DigitalOutputRole;
using ANOUT     = IOPhysicalControllerBase::AnalogOutputRole;

#endif // !IO_PHYSICAL_CONTROLLER_H