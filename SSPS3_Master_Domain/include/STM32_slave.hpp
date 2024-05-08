#pragma once
#ifndef STM32_slave_define
#define STM32_slave_define

#include <Arduino.h>
#include <Wire.h>

using namespace std;

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

enum class COMM_SET : uint8_t
{
    RELAY   = static_cast<uint8_t>(I2C_COMM::SET_RELAY_VAL),
    DAC     = static_cast<uint8_t>(I2C_COMM::SET_DAC_VAL)
};

enum class COMM_GET : uint8_t
{
    RELAY   = static_cast<uint8_t>(I2C_COMM::GET_RELAY_VAL),
    DAC     = static_cast<uint8_t>(I2C_COMM::GET_DAC_VAL),
    DGIN    = static_cast<uint8_t>(I2C_COMM::GET_DGIN_VAL),
    ANIN    = static_cast<uint8_t>(I2C_COMM::GET_ANIN_VAL)
};

class STM32_slave
{
private:
    TwoWire * i2c;
    uint8_t addr;
    uint8_t buf[4];
    uint16_t response;

    void        _set(I2C_COMM cmd, uint8_t pin, uint16_t val);
    uint16_t    _get(I2C_COMM cmd, uint8_t pin);

public:
    STM32_slave(TwoWire * i2c, uint8_t addr, uint8_t sda, uint8_t scl);

    void        set(COMM_SET cmd, uint8_t pin, uint16_t val);
    uint16_t    get(COMM_GET cmd, uint8_t pin);
    uint16_t    get_kb();
}; 

#endif