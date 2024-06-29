#pragma once
#ifndef FRAM_Var_hpp
#define FRAM_Var_hpp

#include <Arduino.h>
#include <Wire.h>
#include "FRAM_crc.hpp"

#define MAX_VARIABLE_LENGTH 100

extern TwoWire * itcw;
extern uint8_t FRAM_address;

void        FRAM_write(uint32_t startAddress, void *data, uint32_t len);
uint32_t    FRAM_read(uint32_t startAddress, void *data, uint32_t len);
uint8_t     FRAM_readByte(uint32_t address);

template <typename T>
class FRAM_Var
{
private:
    uint32_t _address = 0;
    T _defaultValue;

public:
    FRAM_Var(T defaultValue, const uint32_t var_address) :
    _address(var_address), _defaultValue(defaultValue)
    {}

    operator T() {
        return this->get();
    }

    T operator = (T const& value) const
    {
        this->set(value);
        return this->get();
    }

    T operator ++ (int)
    {
        T oldValue = this->get();
        T newValue = oldValue + 1;
        this->set(newValue);
        return oldValue;
    }

    T operator ++ ()
    {
        T newValue = this->get() + 1;
        this->set(newValue);
        return newValue;
    }

    T operator -- (int)
    {
        T oldValue = this->get();
        T newValue = oldValue - 1;
        this->set(newValue);
        return oldValue;
    }

    T operator -- ()
    {
        T newValue = this->get() - 1;
        this->set(newValue);
        return newValue;
    }

    T operator += (T const& value) const
    {
        T newValue = this->get() + value;
        this->set(newValue);
        return newValue;
    }

    T operator -= (T const& value) const
    {
        T newValue = this->get() - value;
        this->set(newValue);
        return newValue;
    }

    T operator *= (T const& value) const
    {
        T newValue = this->get() * value;
        this->set(newValue);
        return newValue;
    }

    T operator /= (T const& value) const
    {
        T newValue = this->get() / value;
        this->set(newValue);
        return newValue;
    }

    T operator %= (T const& value) const
    {
        T newValue = this->get() % value;
        this->set(newValue);
        return newValue;
    }

    T operator &= (T const& value) const
    {
        T newValue = this->get() & value;
        this->set(newValue);
        return newValue;
    }

	T operator |= (T const& value) const
	{
	    T newValue = this->get() | value;
	    this->set(newValue);
	    return newValue;
	}

    bool operator > (T const& value) const {
        return this->get() > value;
    }
    
    bool operator < (T const& value) const {
        return this->get() < value;
    }

    bool operator >= (T const& value) const {
      return this->get() >= value;
    }

    bool operator <= (T const& value) const {
      return this->get() <= value;
    }

    bool operator == (T const& value) const {
      return this->get() == value;
    }

    T get()
    {
        T returnValue;

        if (this->isInitialized())
            FRAM_read(this->_address, &returnValue, sizeof(T));
        else
            returnValue = this->_defaultValue;

      return returnValue;
    }

    void set(T value) 
    {
        FRAM_write(this->_address, &value, sizeof(T));
        uint8_t checksum = FRAM_CRC<T>::get(value);
        FRAM_write(this->checksumAddress(), &checksum, 1);
    }

    bool isInitialized() {
        return (this->checksum() == this->checksumByte());
    }

    uint16_t length() {
        return sizeof(T) + 1; // + 1 crc byte
    }

    void unset(uint8_t unsetValue = 0xff)
    {
        for ( int i = 0; i < this->length(); i++)
            FRAM_write(this->_address + i, &unsetValue, 1);
    }

    uint16_t checksumAddress() {
        return this->_address + this->length() - 1;
    }

    uint16_t checksumByte() {
        return FRAM_readByte(this->checksumAddress());
    }

    uint8_t checksum()
    {
        uint8_t data[MAX_VARIABLE_LENGTH];
        this->copyTo(data, sizeof(T));

        return FRAM_CRC<T>::get(data, sizeof(T));
    }

    void copyTo(uint8_t* data, uint32_t length) {
        FRAM_read(this->_address, data, length);
    }

    uint16_t getAddress() {
        return this->_address;
    }

    T getDefaultValue() {
        return this->_defaultValue;
    }
};

template <>
class FRAM_Var<std::string>
{
private:
    uint32_t _address = 0;
    uint8_t _max_str_len = 0;
    std::string _defaultValue;
    char * buffer;
    char * crc_buffer;

public:
    FRAM_Var(std::string defaultValue, uint8_t max_str_len, const uint32_t var_address) :
    _address(var_address), _max_str_len(max_str_len), _defaultValue(defaultValue)
    {
        buffer = new char[_max_str_len];
        crc_buffer = new char[_max_str_len];

        memset(buffer, 0, _max_str_len);
        memset(crc_buffer, 0, _max_str_len);
    }

    operator std::string() {
        return this->get();
    }

    std::string operator = (const std::string& value)
    {
        this->set(value);
        return this->get();
    }

    std::string get()
    {
        if (this->isInitialized())
        {
            FRAM_read(this->_address, buffer, _max_str_len);
            buffer[_max_str_len - 1] = '\0';
        }
        else
        {
            return this->_defaultValue;
        }

        return std::string(buffer);
    }

    void set(const std::string& value) 
    {
        memset(buffer, 0, _max_str_len);

        strncpy(buffer, value.c_str(), min<uint8_t>(_max_str_len - 1, value.length()));
        buffer[min<uint8_t>(_max_str_len - 1, value.length())] = '\0';

        FRAM_write(this->_address, buffer, _max_str_len);
        uint8_t checksum = FRAM_CRC<std::string>::get((uint8_t*)buffer, _max_str_len);
        FRAM_write(this->checksumAddress(), &checksum, 1);
    }

    bool isInitialized() {
        return (this->checksum() == this->checksumByte());
    }

    uint16_t length() {
        return _max_str_len + 1;  // + 1 crc byte
    }

    void unset(uint8_t unsetValue = 0xff)
    {
        for (int i = 0; i < this->length(); i++)
            FRAM_write(this->_address + i, &unsetValue, 1);
    }

    uint16_t checksumAddress() {
        return this->_address + this->length() - 1;
    }

    uint16_t checksumByte() {
        return FRAM_readByte(this->checksumAddress());
    }

    uint8_t checksum()
    {
        memset(crc_buffer, 0, _max_str_len);
        this->copyTo((uint8_t*)crc_buffer, _max_str_len);
        return FRAM_CRC<std::string>::get((uint8_t*)crc_buffer, _max_str_len);
    }

    void copyTo(uint8_t* data, uint32_t length) {
        FRAM_read(this->_address, data, length);
    }

    uint16_t getAddress() {
        return this->_address;
    }

    std::string getDefaultValue() {
        return this->_defaultValue;
    }
};
#endif