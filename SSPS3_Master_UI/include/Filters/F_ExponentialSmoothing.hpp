#pragma once
#ifndef F_ExponentialSmoothing_hpp
#define F_ExponentialSmoothing_hpp

#include <Arduino.h>
#include "Filter.hpp"

class ExponentialSmoothing : public Filter
{
private:
    float alpha;
    float filtered_value;
    bool initialized;

public:
    ExponentialSmoothing(float alpha) : alpha(alpha), filtered_value(0), initialized(false)
    {}

    void add_value(float value) override
    {
        if (!initialized)
        {
            filtered_value = value * 0.98f;
            initialized = true;
        }
        else
            filtered_value = alpha * value + (1 - alpha) * filtered_value;
    }

    float get_filtered_value() const override {
        return static_cast<float>(filtered_value);
    }
};

#endif