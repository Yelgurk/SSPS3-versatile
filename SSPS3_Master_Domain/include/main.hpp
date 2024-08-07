#include "DateTime/S_DateTime.hpp"
#include "FRAM/FRAM_Storage.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION
#define UNLOCK_DOMAIN_MAIN_CPP

#ifndef main_hpp
#define main_hpp

#include <Arduino.h>

using namespace std;

#define SDA                         48
#define SCL                         47
#define FRAM_I2C_ADDR               0x50

extern uint8_t FRAM_address;
extern TwoWire * itcw;


struct __attribute__((packed)) TaskDataStruct
{
    bool done = 0;
    uint8_t lap = 0;
    double ms_total = 0;
    double ms_left = 0;
    float tempC = 0;

    std::string to_string() {
        return 
        "(" +
        std::to_string(done) + " | " +
        std::to_string(lap) + " | " +
        std::to_string(tempC) + " | " +
        std::to_string(ms_total) + " | " +
        std::to_string(ms_left) +
        ")";
    }
};

TaskDataStruct defaultTaskData;
S_DateTime defaultDT;

static auto& mem_TDS_4 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_TDS_2 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_Timer1 = Storage::allocate<bool>(false);
static auto& mem_Timer2 = Storage::allocate<uint8_t>(255);
static auto& mem_Timer3 = Storage::allocate<uint16_t>(255);
static auto& mem_TDS_1 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_String = Storage::allocate<std::string>("err");
static auto& mem_DT = Storage::allocate<S_DateTime>(defaultDT);
static auto& mem_Timer4 = Storage::allocate<uint32_t>(255);
static auto& mem_Timer5 = Storage::allocate<uint32_t>(255);
static auto& mem_TDS_3 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_TDS_5 = Storage::allocate<TaskDataStruct>(defaultTaskData);
static auto& mem_TDS_6 = Storage::allocate<TaskDataStruct>(defaultTaskData);


#endif
#endif