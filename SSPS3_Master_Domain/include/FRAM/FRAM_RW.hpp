#pragma once
#ifndef FRAM_RW_hpp
#define FRAM_RW_hpp

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <string>
#include <memory>
#include <any>

#define FRAM_I2C_ADDRESS 0x50

extern TwoWire * itcw;
extern uint8_t FRAM_address;

class FRAM
{
public:
    static void fill(uint16_t addr, const uint8_t& filler, size_t size)
    {
        itcw->beginTransmission(FRAM_I2C_ADDRESS);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        for (size_t i = 0; i < size; ++i)
            itcw->write(filler);
        itcw->endTransmission();
    }

    static void write(uint16_t addr, const uint8_t* data, size_t size)
    {
        itcw->beginTransmission(FRAM_I2C_ADDRESS);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        for (size_t i = 0; i < size; ++i)
            itcw->write(*(data + i));
        itcw->endTransmission();
    }

    static void read(uint16_t addr, uint8_t* data, size_t size)
    {
        itcw->beginTransmission(FRAM_I2C_ADDRESS);
        itcw->write((addr >> 8) & 0xFF);
        itcw->write(addr & 0xFF);
        itcw->endTransmission();

        itcw->requestFrom(FRAM_I2C_ADDRESS, size);
        for (size_t i = 0; i < size; ++i)
            if (itcw->available())
                *(data + i) = itcw->read();
    }

    static uint8_t readByte(uint16_t addr)
    {
        uint8_t byte = 0;
        read(addr, &byte, 1);
        return byte;
    }

    static uint8_t calculateCRC(const uint8_t* data, size_t size)
    {
        uint8_t crc = 0;
        for (size_t i = 0; i < size; ++i)
            crc ^= *(data + i);
        return crc;
    }
};

#endif