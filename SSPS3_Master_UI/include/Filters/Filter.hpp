#pragma once
#ifndef Filter_hpp
#define Filter_hpp

#include <Arduino.h>

class Filter
{
public:
    virtual ~Filter() {}
    virtual void add_value(float value) = 0;
    virtual float get_filtered_value() const = 0;
};

#endif