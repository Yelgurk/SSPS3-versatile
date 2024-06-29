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

struct TaskDataStruct
{
    bool done = 0;
    uint8_t lap = 0;
    float tempC = 0;
    double ms_total = 0;
    double ms_left = 0;

    std::string get_str() {
        return 
        "(" +
        to_string(done) + " | " +
        to_string(lap) + " | " +
        to_string(tempC) + " | " +
        to_string(ms_total) + " | " +
        to_string(ms_left) +
        ")";
    }
};

FRAM_Var<boolean>           mem_Timer1(123, FRAM_ALLOC(boolean));
FRAM_Var<uint8_t>           mem_Timer2(123, FRAM_ALLOC_ADDR(uint8_t, 100));
FRAM_Var<uint16_t>          mem_Timer3(123, FRAM_ALLOC(uint16_t));
FRAM_Var<uint32_t>          mem_Timer4(123, FRAM_ALLOC_ADDR(uint32_t, 200));
FRAM_Var<uint32_t>          mem_Timer5(123, FRAM_ALLOC(uint32_t));
FRAM_Var<std::string>       mem_String("default", 10, FRAM_ALLOC(char[10]));
FRAM_Var<TaskDataStruct>    mem_TDS_1(TaskDataStruct(), FRAM_ALLOC(TaskDataStruct));

#endif
#endif