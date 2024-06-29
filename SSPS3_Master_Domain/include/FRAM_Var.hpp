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
#endif