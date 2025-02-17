#pragma once
#ifndef X_VAR_EXTENSION_H
#define X_VAR_EXTENSION_H

#include <cstddef>

static unsigned char extension_calc_crc(const unsigned char* data, size_t size)
{
    unsigned char crc = 0xFF;
    
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= data[i];
        
        for (int bit = 0; bit < 8; ++bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    
    return crc;
}

#define CALC_CRC(data, size)    extension_calc_crc(data, size);

#endif // !MEM_EXTENSION_H
