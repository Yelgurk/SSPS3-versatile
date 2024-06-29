#pragma once
#ifndef FRAM_crc_hpp
#define FRAM_crc_hpp

#include <Arduino.h>

template <typename T>
class FRAM_CRC
{
public:
    static uint8_t get(void* data, uint32_t length)
    {
        uint8_t returnValue = 0;

        if (length == 1)
            returnValue = 0xAA ^ ((uint8_t*)data)[0];
        else
            for (int i = 0; i < length ; i++)
            {
                byte b = ((uint8_t*)data)[i];
                returnValue ^= b;
            }

        if (returnValue == 0xFF)
            returnValue <<= 1;

      return returnValue;
    }
};

#endif