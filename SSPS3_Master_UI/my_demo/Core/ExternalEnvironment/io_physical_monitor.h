#pragma once
#ifndef IO_PHYSICAL_MONITOR_H
#define IO_PHYSICAL_MONITOR_H

#include <Arduino.h>
#include "./io_physical_controller_core.h"
#include "./IOMonitorFilters/filters.h"

#define ADC_TEMP_1_C_TO_12_BIT  8.f
#define ADC_TEMP_C_PRODUCT_OFS  11.f
#define ADC_TEMP_C_WJACKET_OFS  4.f

class IOPhysicalMonitor : public IOPhysicalControllerCore
{
private:
    IOPhysicalMonitor()
    {
        //------------------------
        // Читать ниже про фильтры
        //------------------------

        _f_temp_C_product.add_converter([](short value) {
            return static_cast<float>(value - 1000) / 2.5f;
        });

        _f_temp_C_wJacket.add_converter([](short value) {
            return static_cast<float>(value - 1000) / 2.5f;
        });

        _f_value_24V_batt.add_converter([](short value) {
            return static_cast<float>(value - 1000) / 6.0f;
        });

        //-------------------------------------------------------------
        // В последующем исправить логику конвертеров у фильтров выше
        // относительно как лимитов у конфигов для MPT100 в XStorage,
        // так и относительно offset переменных в define для фикса
        // погрешности датчика (либо вынести эти офсеты в отдельную
        // XStorage переменную?).
        // Кратко: конвертьер для фильтров выше занимаются превращением
        // уже отфильтрованных 12бит занчений АЦП в физическую единицу
        // измерения для дальнейшей работы оборудования и/или вывода
        // ифномрации на экран (в GUI)
        //-------------------------------------------------------------

        add_analog_input_handler(ANIN::TEMP_C_PRODUCT_SENS, [](short pin, short value) {
            IOPhysicalMonitor::instance()->_f_temp_C_product.add_value(value);
        });

        add_analog_input_handler(ANIN::TEMP_C_WJACKET_SENS, [](short pin, short value) {
            IOPhysicalMonitor::instance()->_f_temp_C_wJacket.add_value(value);
        });

        add_analog_input_handler(ANIN::V24_DC_BATT_SENS, [](short pin, short value) {
            IOPhysicalMonitor::instance()->_f_value_24V_batt.add_value(value);
        });

        //-----------------------------------------------------------------
        // Подписки выше "реагируют" на информацию поступающую к master-у
        // со стороный slave-а по факту их изменения. Поэтому частота
        // "считывания" показаний не устанавливается явно со стороны
        // master-а, а вовсе является задачей slave-а. Когда slave отправил
        // данные, тогда только они будут получены и обработаны master-ом.
        // Поэтому задержка опроса аналоговых датчиков по таймерам лежит
        // на slave-а, а потому доделать IOController-а функционал так,
        // что бы его update() методы для аналоговых датчиков опрашивались
        // не непрерывно, а с некоторой частотой + что бы этот idle_ms
        // период был настраиваемым через метод, для управления этими
        // конфигурациями через команды по mqtt_i2c от master-а.
        //----------------------------------------------------------------- 
    }

    IValueTFilter<short, float>& _f_temp_C_product = FilterExponentialSamplingAverage<short, float, 12, 6>().get_i_value_filter(); // unsigned char Alpha will be converted inside into 0.06f => (alpha / 100.f)
    IValueTFilter<short, float>& _f_temp_C_wJacket = FilterExponentialSamplingAverage<short, float, 12, 6>().get_i_value_filter(); // unsigned char Alpha will be converted inside into 0.06f => (alpha / 100.f)
    IValueTFilter<short, float>& _f_value_24V_batt = FilterExponentialSmoothing<short, float>(0.03f).get_i_value_filter(); 

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

    float get_temperature_C_product() {
        return _f_temp_C_product.get_value_converted();
    }

    float get_temperature_C_wjacket() {
        return _f_temp_C_wJacket.get_value_converted();
    }

    float get_battery_24V_charge() {
        return _f_value_24V_batt.get_value_converted();
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
        this->change_digital_output_pin_state(DOUT::MIXER_RELAY, rpm > 0);
        this->change_digital_output_pin_state(DOUT::MIXER_FAST_MODE_RELAY, is_fast_mode);

        //-------------------------------------------------------
        // Тут логитка работы с XStorage и переменными
        // отвечающими за 12bit max/min limit пороговые значения
        // (12bit max/min для DAC нужны, т.к. в старых версиях
        // max 12bit значение 4095 по схеме выдавало 9.8V, а в
        // выполнен перерасчёт номиналов резисторов для 
        // операционных уселителей и в новых ПЛК DAC при 4095
        // выдаёт до 12V. Так что лимиты нужны для расчётов, что
        // бы не сжечь подключаемые девайсы)
        // и что они обозначающие как для обычной мешалки,
        // так и для is_fast_mode.
        // Выполнить рассчёты и затем только передавать значение,
        // но не rpm, а уже 12бит значение.
        //-------------------------------------------------------
        this->set_analog_output_info(ANOUT::RPM_SPEED_DRIVER, rpm);
    }
};

#define IOMonitor    IOPhysicalMonitor::instance()

#endif