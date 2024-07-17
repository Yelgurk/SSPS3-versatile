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
    deque<uint16_t> values;
    size_t max_size;
    
public:
    MovingAverage(size_t size) : max_size(size) {}
    
    void add_value(uint16_t value) override
    {
        values.push_back(value);

        if (values.size() > max_size)
            values.pop_front();
    }
    
    uint16_t get_filtered_value() const override
    {
        if (values.empty()) return 0;

        uint32_t sum = 0;
        for (uint16_t v : values)
            sum += v;

        return sum / values.size();
    }
};

#endif