#pragma once
#ifndef X_VAR_EXTENSION_H
#define X_VAR_EXTENSION_H

#include <cstddef>

static unsigned char extension_calc_crc(const unsigned char* data, size_t size)
{
    static unsigned char crc = 0;
    crc = 0;

    for (size_t i = 0; i < size; ++i)
        crc ^= *(data + i);

    return crc;
}

#define CALC_CRC(data, size)    extension_calc_crc(data, size);

#endif // !MEM_EXTENSION_H
