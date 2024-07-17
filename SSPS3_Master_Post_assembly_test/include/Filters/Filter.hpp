#pragma once
#ifndef Filter_hpp
#define Filter_hpp

#include <Arduino.h>

class Filter
{
public:
    virtual ~Filter() {}
    virtual void add_value(uint16_t value) = 0;
    virtual uint16_t get_filtered_value() const = 0;
};

#endif