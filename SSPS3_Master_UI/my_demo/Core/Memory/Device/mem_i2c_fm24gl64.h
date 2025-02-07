#pragma once
#ifndef MEM_I2C_FM24GL64_H
#define MEM_I2C_FM24GL64_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <cstddef>
    #include "../Interface/i_read_write.h"
    #include "../Interface/i_arduino_i2c.h"
    #include "Wire.h"

    typedef TwoWire         I2C_CH;
#else
    #include <cstddef>
    #include "./i_read_write.h"
    #include "./i_arduino_i2c.h"

    typedef IArduinoI2C     I2C_CH;
#endif

class FM24GL64 : public IReadWrite
{
private:
    static I2C_CH* itcw;
    unsigned char device_i2c_addr = 0x50;

public:
    FM24GL64(unsigned char _device_i2c_addr = 0x50)
    : device_i2c_addr(_device_i2c_addr)
    {}

    static void set_i2c_ch(I2C_CH* _itcw)
    {
        FM24GL64::itcw = _itcw;
    }
    
    void fill(unsigned int addr, const unsigned char& filler, size_t size) override
    {
        itcw->beginTransmission(this->device_i2c_addr);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        for (size_t i = 0; i < size; ++i)
            itcw->write(filler);
        itcw->endTransmission();
    }

    bool read(unsigned int addr, unsigned char* data, size_t size, bool check_last_short_crc = false) override
    {
        itcw->beginTransmission(this->device_i2c_addr);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        itcw->endTransmission();

        itcw->requestFrom(this->device_i2c_addr, size);
        for (size_t i = 0; i < size; ++i)
            if (itcw->available())
                *(data + i) = itcw->read();

        return false;
    }

    void write(unsigned int addr, const unsigned char* data, size_t size) override
    {
        itcw->beginTransmission(this->device_i2c_addr);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        for (size_t i = 0; i < size; ++i)
            itcw->write(*(data + i));
        itcw->endTransmission();
    }

    char byte_read(unsigned int addr) override
    {
        unsigned char byte = 0;
        this->read(addr, &byte, 1);
        return byte;
    }
};

I2C_CH* FM24GL64::itcw = NULL;

#endif // !MEM_I2C_FM24GL64_H
