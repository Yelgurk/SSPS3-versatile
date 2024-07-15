#pragma once
#ifndef FRAM_Object_hpp
#define FRAM_Object_hpp

#include <Arduino.h>
#include <Wire.h>
#include <map>
#include <string>
#include <memory>
#include <any>

#include "FRAM_RW.hpp"

extern uint8_t FRAM_address;
extern TwoWire * itcw;

class FRAMObjectBase
{
public:
    virtual ~FRAMObjectBase() = default;
};

template<typename T>
class FRAMObject : public FRAMObjectBase
{
private:
    uint16_t address;
    uint16_t var_size;
    uint16_t full_size;
    uint8_t * _buffer;
    T _value;
    T _default_value;

public:
    FRAMObject(uint16_t addr, T default_value, uint16_t size)
    : address(addr), _default_value(default_value), var_size(size), full_size(size + 1)
    {
        _buffer = new uint8_t[full_size];
    }

    uint16_t get_addr() {
        return this->address;
    }

    uint8_t crc_calc_local() {
        return FRAM::calculateCRC(_buffer, var_size);
    }

    uint8_t crc_calc_fram()
    {
        FRAM::read(address, _buffer, full_size);
        return FRAM::calculateCRC(_buffer, var_size);
    }

    uint8_t crc_fram() {
        return FRAM::readByte(address + var_size);
    }

    bool crc_state()
    {
        if (crc_calc_local() == crc_fram())
            return true;
        else
            return crc_calc_fram() == crc_fram();
    }
    
    void unset(uint8_t unset_val = 0x00) {
        FRAM::fill(address, unset_val, full_size);
    }

    void reset() {
        set(_default_value);
    }

    void set(const T& value)
    {
        memcpy(_buffer, &value, var_size);
        _buffer[var_size] = FRAM::calculateCRC(_buffer, var_size);
        FRAM::write(address, _buffer, full_size);
        memcpy(&_value, _buffer, var_size);
    }

    void accept() {
        set(_value);
    }

    void cancel() {
        crc_calc_fram();
    }

    T get()
    {
        if (crc_state())
        {
            memcpy(&_value, _buffer, var_size);
            return _value;
        }
        else
        {
            //unset();
            reset();
            return _value;
        }
    }

    T* ptr()
    {
        if (crc_state())
        {
            memcpy(&_value, _buffer, var_size);
            return &_value;
        }
        else
        {
            //unset();
            reset();
            return &_value;
        }
    }

    operator T () {
        return this->get();
    }
};

template<>
class FRAMObject<std::string> : public FRAMObjectBase
{
private:
    uint16_t address;
    uint16_t var_size;
    uint16_t full_size;
    uint8_t * _buffer;
    std::string _value;
    std::string _default_value;

    void clear_buff() {
        memset(_buffer, 0, full_size);
    }

public:
    FRAMObject(uint16_t addr, std::string default_value, uint16_t size)
    : address(addr), _default_value(default_value), var_size(size), full_size(size + 1)
    {
        _buffer = new uint8_t[full_size];
    }

    uint16_t get_addr() {
        return this->address;
    }

    uint8_t crc_calc()
    {
        clear_buff();
        FRAM::read(address, _buffer, full_size);
        return FRAM::calculateCRC(_buffer, var_size);
    }

    uint8_t crc_fram() {
        return FRAM::readByte(address + var_size);
    }

    bool crc_state() {
        return crc_calc() == crc_fram();
    }
    
    void unset(uint8_t unset_val = 0x00) {
        FRAM::fill(address, unset_val, full_size);
    }

    void reset() {
        set(_default_value);
    }

    void set(const std::string& value)
    {
        clear_buff();
        memcpy(_buffer, value.c_str(), min<uint16_t>(var_size - 1, value.length()));

        _buffer[var_size] = FRAM::calculateCRC(_buffer, var_size);
        FRAM::write(address, _buffer, full_size);
    }

    std::string get()
    {
        if (crc_state())
            return _value.assign((char*)_buffer, var_size);
        else
        {
            //unset();
            reset();
            return _default_value;
        }
    }

    operator std::string () {
        return this->get();
    }

    operator const char *() {
        return this->get().c_str();
    }
};

#endif