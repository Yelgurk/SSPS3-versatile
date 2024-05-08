#pragma once
#ifndef FRAM_crc_hpp
#define FRAM_crc_hpp

#include <Arduino.h>

template <typename T>
class FRAM_CRC
{
public:
    static uint8_t get(uint8_t* data, uint32_t length)
    {
        uint8_t returnValue = 0;

        if (length == 1)
            returnValue = 0xAA ^ data[0];
        else
            for (int i = 0; i < length ; i++)
            {
                byte b = data[i];
                returnValue ^= b;
            }

        if (returnValue == 0xFF)
            returnValue <<= 1;

      return returnValue;
    }

    static uint8_t get(T value)
    {
        uint8_t *ptr = (uint8_t*) &value;
        return FRAM_CRC<T>::get(ptr, sizeof(T));
    }
};

#endif