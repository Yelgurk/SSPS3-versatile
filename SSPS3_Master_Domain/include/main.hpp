#pragma once
#ifndef main_hpp
#define main_hpp

#include <Arduino.h>
#include "FRAM_Var.hpp"
#include "FRAM_allocator.hpp"
#include "DateTime/S_DateTime.hpp"

#define FRAM_ALLOC(type)            FRAM_allocator::getAddr(sizeof(type))
#define FRAM_ALLOC_ADDR(type, addr) FRAM_allocator::getAddr(sizeof(type), addr)

#ifndef SSPS3_APPLICATION_SOLUTION

using namespace std;

#define SDA                         48
#define SCL                         47
#define FRAM_I2C_ADDR               0x50

extern uint8_t FRAM_address;
extern TwoWire * itcw;

template <size_t N>
struct FRAM_str
{
    char char_arr[N];

    FRAM_str()
    {
        memset(char_arr, 0, N);
    }

    FRAM_str(const char* str)
    {
        strncpy(char_arr, str, N - 1);
        char_arr[N - 1] = '\0';
    }

    FRAM_str * set_str(const char* str)
    {
        strncpy(char_arr, str, N - 1);
        char_arr[N - 1] = '\0';

        return this;
    }

    const char* get_str() const
    {
        return char_arr;
    }

    operator const char*() const
    {
        return get_str();
    }

    FRAM_str& operator=(const char* str)
    {
        set_str(str);
        return *this;
    }
};

FRAM_Var<boolean>       mem_Timer1(123, FRAM_ALLOC(boolean));
FRAM_Var<uint8_t>       mem_Timer2(123, FRAM_ALLOC(uint8_t));
FRAM_Var<uint16_t>      mem_Timer3(123, FRAM_ALLOC(uint16_t));
FRAM_Var<uint32_t>      mem_Timer4(123, FRAM_ALLOC_ADDR(uint32_t, 100));
FRAM_Var<uint32_t>      mem_Timer5(123, FRAM_ALLOC(uint32_t));

#endif
#endif