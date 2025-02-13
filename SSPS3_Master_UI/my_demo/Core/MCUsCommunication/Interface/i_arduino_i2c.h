#pragma once
#ifndef I_ARDUINO_I2C_H
#define I_ARDUINO_I2C_H

#ifdef DEV_SSPS3_RUN_ON_PLC

#include "./i_arduino_io.h"
#include "Wire.h"

typedef TwoWire         I2C_CH;

#else

#include "./i_arduino_io.h"
#include <functional>

class IArduinoI2C
{
public:
    IArduinoI2C() {}
    IArduinoI2C(uint8_t bus_num) {}

    typedef std::function<void(int)> cb_function_receive_t;
    typedef std::function<void(void)> cb_function_request_t;

    void onReceive(cb_function_receive_t function) {}
    void onRequest(cb_function_request_t function) {}

    bool begin()                                                            { return false; }
    bool begin(uint8_t addr)                                                { return false; }
    bool begin(int sda, int scl, uint32_t frequency = 0)                    { return false; }
    bool begin(uint8_t slave_addr, int sda, int scl, uint32_t frequency)    { return false; }

    void beginTransmission(char i2c_device_addr) {}

    uint8_t requestFrom(char i2c_device_addr, size_t size)  { return 0; }

    char read()                                             { return 0x00; }
    size_t readBytes(uint8_t* buffer, size_t length)        { return 0; }

    void write(char byte) {}
    void write(const uint8_t *byte, size_t size) {}

    void endTransmission() {}

    bool available() { return true; }
};

typedef IArduinoI2C     I2C_CH;

static I2C_CH Wire = I2C_CH();

#endif

#endif // !I_ARDUINO_I2C_H
