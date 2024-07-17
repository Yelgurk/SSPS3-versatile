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
    Filter* currentFilter;
    uint16_t minValue;
    uint16_t maxValue;
    double physicalMin;
    double physicalMax;

public:
    FilterValue(Filter* filter, uint16_t minVal = 0, uint16_t maxVal = 4095, double physMin = 0.0, double physMax = 100.0)
        : currentFilter(filter), minValue(minVal), maxValue(maxVal), physicalMin(physMin), physicalMax(physMax) {}

    void set_filter(Filter* filter) {
        currentFilter = filter;
    }

    void set_min_max_values(uint16_t minVal, uint16_t maxVal) {
        minValue = minVal;
        maxValue = maxVal;
    }

    void set_physical_min_max(double physMin, double physMax) {
        physicalMin = physMin;
        physicalMax = physMax;
    }

    void add_value(uint16_t value) {
        currentFilter->add_value(value);
    }

    uint16_t get_filtered_value() {
        return currentFilter->get_filtered_value();
    }

    uint16_t get_scaled_value() {
        uint16_t filteredValue = get_filtered_value();
        return map(filteredValue, 0, 4095, minValue, maxValue);
    }

    double get_percentage_value() {
        uint16_t scaledValue = get_scaled_value();
        return (static_cast<double>(scaledValue - minValue) / (maxValue - minValue)) * 100.0;
    }

    double get_physical_value() {
        uint16_t filteredValue = get_filtered_value();
        return physicalMin + ((static_cast<double>(filteredValue) / 4095) * (physicalMax - physicalMin));
    }

    long map(long x, long in_min, long in_max, long out_min, long out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   
};

#endif