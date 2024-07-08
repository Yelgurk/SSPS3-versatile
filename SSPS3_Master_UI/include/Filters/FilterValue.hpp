#pragma once
#ifndef FilterValue_hpp
#define FilterValue_hpp

#include <Arduino.h>
#include "F_ExponentialSmoothing.hpp"
#include "F_MedianFilter.hpp"
#include "F_MovingAverage.hpp"

class FilterValue
{
private:
    Filter* current_filter;
    uint16_t min_value;
    uint16_t max_value;

public:
    FilterValue(Filter* filter, uint16_t min_value = 0, uint16_t max_value = 4095)
        : current_filter(filter), min_value(min_value), max_value(max_value) {}

    void set_filter(Filter* filter) {
        current_filter = filter;
    }

    void set_min_max(uint16_t min_value, uint16_t max_value) {
        this->min_value = min_value;
        this->max_value = max_value;
    }

    uint16_t get_filtered_value() const {
        return current_filter->get_filtered_value();
    }

    uint16_t get_filtered_value_relative() const {
        uint16_t filtered = current_filter->get_filtered_value();
        return static_cast<uint16_t>((filtered - min_value) * 4095 / (max_value - min_value));
    }

    double get_filtered_value_percentage() const {
        uint16_t filtered = current_filter->get_filtered_value();
        return static_cast<double>(filtered - min_value) * 100 / (max_value - min_value);
    }

    void add_value(uint16_t value) {
        current_filter->add_value(value);
    }
};

#endif