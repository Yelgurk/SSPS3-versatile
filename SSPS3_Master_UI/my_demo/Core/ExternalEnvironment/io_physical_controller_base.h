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

    unsigned char const available_digital_in_pins = 5;
    enum DigitalInputRole : unsigned char
    {
        DISPENSER_SIGNAL    = 0,
        V380_SIGNAL         = 1,
        MIXER_ERROR_SIGNAL  = 2,
        STOP_BTN_SIGNAL     = 3,
        WATER_JACKET_SIGNAL = 4,
        FREE_5              = 5,
        FREE_6              = 6,
        FREE_7              = 7,
        DIGITAL_INPUT_END   = 0b00001111
    };

    unsigned char const available_analog_in_pins = 3;
    enum AnalogInputRole : unsigned char
    {
        TEMP_C_PROFUCT_SENS = 0,
        TEMP_C_WJACKET_SENS = 1,
        v24_DC_BATT_SENS    = 2,
        FREE_3              = 3,
        ANALOG_INPUT_END    = 0b00001111
    };

    unsigned char const available_digital_out_pins = 4;
    enum DigitalOutputRole : unsigned char
    {
        HEATERS_RELAY       = 0,
        MIXER_RELAY         = 1,
        WJACKET_RELAY       = 2,
        WATER_PUMP_RELAY    = 3,
        FREE_4              = 4,
        DIGITAL_OUTPUT_END  = 0b00001111
    };

    unsigned char const available_analog_out_pins = 1;
    enum AnalogOutputRole : unsigned char
    {
        RPM_SPEED_DRIVER    = 0,
        ANALOG_OUTPUT_END   = 0b00001111
    };

private:
    unsigned char _digital_input    = 0b00000000;   // доступно 5 цифровых входов. Каждый бит как элемент массива [0...7]
    unsigned char _digital_output   = 0b00000000;   // доступно 4 цифровых выходов. Каждый бит как элемент массива [0...7]
    short _analog_input[3]          = { 0 };        // доступно 3 аналоговых 12бит входов
    short _analog_output[1]         = { 0 };        // доступен 1 аналоговый 12бит выход

    std::vector<std::pair<unsigned char, IODigitalSignalHandler>> _digital_signal_handlers;
    std::vector<std::pair<unsigned char, IOAnalogSignalHandler>> _analog_signal_handlers;

    unsigned char const _channel_mask   = 0b11110000;
    unsigned char const _pin_index_mask = 0b00001111;

protected:
    IOPhysicalControllerBase()
    {}

    bool _digital_io_signals_comparator(unsigned char _old, unsigned char _new, unsigned char _index)
    {
        bool oldBit = ((_old & (1 << _index)) != 0);
        bool newBit = ((_new & (1 << _index)) != 0);
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

    unsigned char _formulate_instructions(unsigned char _channel_enum, unsigned char _pin_role_enum) {
        return (0b00000000 | _channel_enum | _pin_role_enum);
    }

    // 2 extra метода, на всякий случай, для извлечения номера пина или самого канала
    unsigned char _get_pin_index_from_instruction(unsigned char instruction) {
        return (instruction & _pin_index_mask);
    }

    Channel _get_channel_from_instruction(unsigned char instruction) {
        return static_cast<Channel>(instruction & _channel_mask);
    }

public:
    static IOPhysicalControllerBase* instance_base()
    {
        static IOPhysicalControllerBase inst;
        return &inst;
    }

    void set_digital_input_info(unsigned char digital_input_info)
    {
        for (unsigned char index = 0; index < available_digital_in_pins; index++)
        {
            bool _is_equal = _digital_io_signals_comparator(_digital_input, digital_input_info, index); 

            if (!_is_equal)
            {
                unsigned char _instr = _formulate_instructions(Channel::DIGITAL_INPUT, index);
                bool _new_state = _digital_io_signal_get_bit(digital_input_info, index);

                for (auto &handler_pair : _digital_signal_handlers)
                {
                    if (handler_pair.first == _instr)
                        handler_pair.second(_new_state);
                }
            }
        }

        this->_digital_input = digital_input_info;
    }

    void set_digital_output_info(unsigned char digital_output_info) {
        this->_digital_output = digital_output_info;
    }

    void set_analog_input_info(short analog_input_info, unsigned char index)
    {
        if (index >= available_analog_in_pins)
            return;

        if (analog_input_info != _analog_input[index])
        {
            unsigned char _instr = _formulate_instructions(Channel::ANALOG_INPUT, index);

            for (auto &handler_pair : _analog_signal_handlers)
            {
                if (handler_pair.first == _instr)
                    handler_pair.second(analog_input_info);
            }
        }

        _analog_input[index] = analog_input_info;
    }

    void set_analog_output_info(short analog_output_info, unsigned char index)
    {
        if (index >= available_analog_out_pins)
            return;

        _analog_output[index] = analog_output_info;
    }

    void change_digital_output_state(bool new_state, unsigned char pin_enum)
    {
        if (pin_enum >= available_digital_out_pins)
            return;

        bool _is_equal = _digital_io_signal_get_bit(this->_digital_output, pin_enum) == new_state;

        if (!_is_equal)
        {
            _digital_io_signal_change_bit(this->_digital_output, pin_enum, new_state);

            unsigned char _instr = _formulate_instructions(Channel::DIGITAL_OUTPUT, pin_enum);

            for (auto &handler_pair : _digital_signal_handlers)
            {
                if (handler_pair.first == _instr)
                    handler_pair.second(new_state);
            }
        }
    }

    void change_digital_output_states_all(bool new_state)
    {
        for (unsigned char index = 0; index < available_digital_out_pins; index++)
            change_digital_output_state(new_state, index);
    }

    void change_analog_output_state(short new_value, unsigned char pin_enum)
    {
        if (pin_enum >= available_analog_out_pins)
            return;

        bool _is_equal = _analog_output[pin_enum] == new_value;

        if (!_is_equal)
        {
            _analog_output[pin_enum] = new_value;

            unsigned char _instr = _formulate_instructions(Channel::ANALOG_OUTPUT, pin_enum);

            for (auto &handler_pair : _analog_signal_handlers)
            {
                if (handler_pair.first == _instr)
                    handler_pair.second(new_value);
            }
        }
    }

    void add_digital_input_handler(DigitalInputRole pin, IODigitalSignalHandler handler) {
        _digital_signal_handlers.push_back({ _formulate_instructions(Channel::DIGITAL_INPUT, pin), handler});
    }

    void add_analog_input_handler(AnalogInputRole pin, IOAnalogSignalHandler handler) {
        _analog_signal_handlers.push_back({ _formulate_instructions(Channel::ANALOG_INPUT, pin), handler});
    }

    void add_digital_output_handler(DigitalOutputRole pin, IODigitalSignalHandler handler) {
        _digital_signal_handlers.push_back({ _formulate_instructions(Channel::DIGITAL_OUTPUT, pin), handler});
    }

    void add_analog_output_handler(AnalogOutputRole pin, IOAnalogSignalHandler handler) {
        _analog_signal_handlers.push_back({ _formulate_instructions(Channel::ANALOG_OUTPUT, pin), handler});
    }
};

#endif // !IO_PHYSICAL_CONTROLLER_H