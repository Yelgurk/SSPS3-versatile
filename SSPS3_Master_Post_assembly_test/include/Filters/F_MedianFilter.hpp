#pragma once
#ifndef F_MedianFilter_hpp
#define F_MedianFilter_hpp

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "Filter.hpp"

using namespace std;

class MedianFilter : public Filter
{
private:
    std::vector<uint16_t> values;
    size_t max_size;
    
public:
    MedianFilter(size_t size) : max_size(size) {}
    
    void add_value(uint16_t value) override
    {
        values.push_back(value);

        if (values.size() > max_size)
            values.erase(values.begin());
    }
    
    uint16_t get_filtered_value() const override
    {
        if (values.empty()) return 0;

        vector<uint16_t> sorted_values = values;
        sort(sorted_values.begin(), sorted_values.end());
        
        return sorted_values[sorted_values.size() / 2];
    }
};

#endif