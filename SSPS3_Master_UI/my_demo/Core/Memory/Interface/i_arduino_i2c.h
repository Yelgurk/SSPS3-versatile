#pragma once
#ifndef I_ARDUINO_I2C_H
#define I_ARDUINO_I2C_H

#ifndef DEV_SSPS3_RUN_ON_PLC

class IArduinoI2C
{
public:
    void beginTransmission(char i2c_device_addr) {}
    char read() { return 0x00; }
    void write(char byte) {}
    void endTransmission() {}
    void requestFrom(char i2c_device_addr, size_t size) {}
    bool available() { return true; }
};

#endif

#endif // !I_ARDUINO_I2C_H
