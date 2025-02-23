#pragma once
#ifndef PIN_MAP_F103RCT6_V2_H
#define PIN_MAP_F103RCT6_V2_H

#include "./Interface/i_pin_map.h"
#include "./PinReg/pin_reg_f103rct6.h"

class PinMap_F103RCT6_V2 : public IPinMap
{
private:
    PinMap_F103RCT6_V2()
    {
        for (auto pin : din_array)
            pinMode(pin, INPUT_PULLDOWN);

        for (auto pin : dout_array)
            pinMode(pin, OUTPUT);
        
        for (auto pin : anin_array)
            pinMode(pin, INPUT);

        for (auto pin : anout_array)
            pinMode(pin, OUTPUT);
    }

    enum DIN_MAP : unsigned char
    {
        DIN_1 = PB5,
        DIN_2 = PB4,
        DIN_3 = PB3,
        DIN_4 = PD2,
        DIN_5 = PC12,
        DIN_6 = PC11,
        DIN_7 = PC10,
        DIN_8 = PA15
    };

    enum DOUT_MAP : unsigned char
    {
        DOUT_1 = PB1,
        DOUT_2 = PB10,
        DOUT_3 = PB12,
        DOUT_4 = PB15,
        DOUT_5 = PA12,
        DOUT_6 = PA10,
        DOUT_7 = PA8,
        DOUT_8 = PC8
    };

    enum ANIN_MAP : unsigned char
    {
        ANIN_1 = PC3,
        ANIN_2 = PC2,
        ANIN_3 = PC1,
        ANIN_4 = PC0
    };

    enum ANOUT_MAP : unsigned char
    {
        ANOUT_1 = PA4
    };

    enum DIN : unsigned char
    {
        DISPENSER_SIGNAL    = DIN_MAP::DIN_1, 
        V380_SIGNAL         = DIN_MAP::DIN_2,
        MIXER_ERROR_SIGNAL  = DIN_MAP::DIN_3,
        STOP_BTN_SIGNAL     = DIN_MAP::DIN_4,
        WATER_JACKET_SIGNAL = DIN_MAP::DIN_5,
        DIN_COUNT = 5
    };

    enum DOUT : unsigned char
    {
        HEATERS_RELAY           = DOUT_MAP::DOUT_1,         
        MIXER_RELAY             = DOUT_MAP::DOUT_2,
        WJACKET_RELAY           = DOUT_MAP::DOUT_3,
        WATER_PUMP_RELAY        = DOUT_MAP::DOUT_4,
        MIXER_FAST_MODE_RELAY   = DOUT_MAP::DOUT_5,
        DOUT_COUNT = 5
    };

    enum ANIN : unsigned char
    {
        TEMP_C_PROFUCT_SENS = ANIN_MAP::ANIN_1, 
        TEMP_C_WJACKET_SENS = ANIN_MAP::ANIN_2,
        V24_DC_BATT_SENS    = ANIN_MAP::ANIN_3,
        ANIN_COUNT = 3
    };

    enum ANOUT : unsigned char
    {
        RPM_SPEED_DRIVER    = ANOUT_MAP::ANOUT_1,
        ANOUT_COUNT = 1
    };

public:
    static PinMap_F103RCT6_V2* instance()
    {
        static PinMap_F103RCT6_V2 inst;
        return &inst;
    }

    // Массивы, хранящие маппинг, должны быть проинициализированы в порядке
    // согласно DIN, DOUT, ANIN, ANOUT enum-ам "виртуальной" распиновки
    // в файле io_physical_controller_core.h
    unsigned char din_array[DIN_COUNT]
    {
        DISPENSER_SIGNAL,
        V380_SIGNAL,  
        MIXER_ERROR_SIGNAL, 
        STOP_BTN_SIGNAL,    
        WATER_JACKET_SIGNAL
    };

    unsigned char dout_array[DOUT_COUNT]
    {
        HEATERS_RELAY,        
        MIXER_RELAY,          
        WJACKET_RELAY,        
        WATER_PUMP_RELAY,     
        MIXER_FAST_MODE_RELAY
    };

    unsigned char anin_array[ANIN_COUNT]
    {
        TEMP_C_PROFUCT_SENS,
        TEMP_C_WJACKET_SENS,
        V24_DC_BATT_SENS   
    };

    unsigned char anout_array[ANOUT_COUNT]
    {
        RPM_SPEED_DRIVER
    };

    //------------------------
    // Virtual to physical pin
    //------------------------

    virtual short _vtp_din(unsigned char virtual_pin) override {
        return this->_get_value_by_index(din_array, DIN_COUNT, virtual_pin);
    }

    virtual short _vtp_dout(unsigned char virtual_pin) override {
        return this->_get_value_by_index(dout_array, DOUT_COUNT, virtual_pin);
    }
    
    virtual short _vtp_anin(unsigned char virtual_pin) override {
        return this->_get_value_by_index(anin_array, ANIN_COUNT, virtual_pin);
    }
    
    virtual short _vtp_anout(unsigned char virtual_pin) override {
        return this->_get_value_by_index(anout_array, ANOUT_COUNT, virtual_pin);
    }
    
    //------------------------
    // Physical to virtual pin
    //------------------------

    virtual short _ptv_din(unsigned char physical_pin) override {
        return this->_get_index_in_array(din_array, DIN_COUNT, physical_pin);
    }
    
    virtual short _ptv_dout(unsigned char physical_pin) override {
        return this->_get_index_in_array(dout_array, DOUT_COUNT, physical_pin);
    }
    
    virtual short _ptv_anin(unsigned char physical_pin) override {
        return this->_get_index_in_array(anin_array, ANIN_COUNT, physical_pin);
    }
    
    virtual short _ptv_anout(unsigned char physical_pin) override {
        return this->_get_index_in_array(anout_array, ANOUT_COUNT, physical_pin);
    }
};

#endif // !PIN_MAP_F103RCT6_V2_H