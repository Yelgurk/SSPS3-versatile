#pragma once
#ifndef I_PIN_MAP_H
#define I_PIN_MAP_H

#include <Arduino.h>
#include <cstring>

class IPinMap
{
protected:
    short _get_value_by_index(unsigned char* _arr, unsigned char _size, unsigned char _index)
    {
        if (_index >= _size)
            return -1;
        
        return *(_arr + _index);
    }
    
    short _get_index_in_array(unsigned char* _arr, unsigned char _size, unsigned char _value)
    {
        void* result = std::memchr(_arr, _value, _size);
    
        if (result != nullptr)
            return static_cast<unsigned char*>(result) - _arr; // Вычисляем индекс как разницу указателей
        else
            return -1;
    }

public:
    IPinMap()
    {}

    // convert_virtual_to_physical_pin  => (c)vtp(p)
    // virtual din to physical din      => (c)vtp(p)_din
    // virtual dout to physical dout    => (c)vtp(p)_dout
    // ... and so on
    virtual short _vtp_din(unsigned char virtual_pin)     = 0;
    virtual short _vtp_dout(unsigned char virtual_pin)    = 0;
    virtual short _vtp_anin(unsigned char virtual_pin)    = 0;
    virtual short _vtp_anout(unsigned char virtual_pin)   = 0;

    // convert_physical_to_virtual_pin  => (c)ptv(p)
    // physical din to virtual din      => (c)ptv(p)_din
    // physical dout to virtual dout    => (c)ptv(p)_dout
    // ... and so on
    virtual short _ptv_din(unsigned char physical_pin)   {}
    virtual short _ptv_dout(unsigned char physical_pin)  {}
    virtual short _ptv_anin(unsigned char physical_pin)  {}
    virtual short _ptv_anout(unsigned char physical_pin) {}

    IPinMap* get_i_pin_map () {
        return this;
    }
};

#endif // !I_PIN_MAP_H