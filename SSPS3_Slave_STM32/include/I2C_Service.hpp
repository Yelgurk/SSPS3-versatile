#pragma once
#ifndef I2C_Service_define
#define I2C_Service_define

#include <Arduino.h>
#include <Wire.h>

using namespace std;

#define MASTER_INT_DEL          500
#define MASTER_AWAIT_LIMIT_MS   2000



enum class I2C_COMM : uint8_t
{
    SET_BEGIN,
    SET_RELAY_VAL,
    SET_DAC_VAL,
    SET_END,

    GET_BEGIN,
    GET_RELAY_VAL,
    GET_DAC_VAL,
    GET_DGIN_VAL,
    GET_ANIN_VAL,
    GET_KB_VAL,
    GET_END
};

class I2C_Service
{
private:
    TwoWire * i2c;
    uint8_t addr;
    uint8_t buf[4];
    uint16_t response;

    I2C_COMM command;
    uint8_t pin;
    uint16_t val;

    function<void(I2C_COMM, uint8_t, uint16_t)> set_event;
    function<uint16_t(I2C_COMM, uint8_t)> get_event;

    
    void is_master_runned_check();

public:
    uint32_t volatile i2c_last_call_ms = 0,
                      i2c_silent_cnt = 0;
    bool volatile i2c_master_runned = false;

    I2C_Service(
        TwoWire * i2c,
        uint8_t addr,
        function<void(I2C_COMM, uint8_t, uint16_t)> set_event,
        function<uint16_t(I2C_COMM, uint8_t)> get_event
        );
}; 

#endif