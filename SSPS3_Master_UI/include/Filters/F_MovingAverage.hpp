#pragma once
#ifndef F_MovingAverage_hpp
#define F_MovingAverage_hpp

#include <Arduino.h>
#include <deque>
#include "Filter.hpp"

using namespace std;

class MovingAverage : public Filter
{
private:
    deque<float> values;
    size_t max_size;
    
public:
    MovingAverage(size_t size) : max_size(size) {}
    
    void add_value(float value) override
    {
        values.push_back(value);

        if (values.size() > max_size)
            values.pop_front();
    }
    
    float get_filtered_value() const override
    {
        if (values.empty()) return 0;

        float sum = 0;
        for (float v : values)
            sum += v;

        return sum / (float)values.size();
    }
};

#endif