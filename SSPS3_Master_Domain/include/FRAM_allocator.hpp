#pragma once
#ifndef FRAM_allocator_hpp
#define FRAM_allocator_hpp

#include <Arduino.h>

class FRAM_allocator
{
public:
    static uint32_t currAddress;
    static uint16_t prevSize;

    static uint32_t getAddr(uint16_t size, int32_t countdown = -1)
    {
        if (size > 1)
            currAddress += 1;

        if (countdown < 0)
            currAddress += (size + 1);
        else
            currAddress = (countdown > currAddress ? countdown : currAddress) + (size + 1);
        
        prevSize = size;
        return currAddress - (size + 1);
    }
};

uint32_t FRAM_allocator::currAddress = 0;
uint16_t FRAM_allocator::prevSize = 0;

#endif