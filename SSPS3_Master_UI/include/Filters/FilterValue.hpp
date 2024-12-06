#pragma once
#ifndef FilterValue_hpp
#define FilterValue_hpp

#include <Arduino.h>
#include "F_ExponentialSmoothing.hpp"
#include "F_MedianFilter.hpp"
#include "F_MovingAverage.hpp"

class FilterValue
{
public:
    Filter* currentFilter;
    int16_t minValue;
    int16_t maxValue;
    float physicalMin;
    float physicalMax;

public:
    FilterValue(Filter* filter, int16_t minVal = 0, int16_t maxVal = 4095, float physMin = 0.0, float physMax = 100.0)
        : currentFilter(filter), minValue(minVal), maxValue(maxVal), physicalMin(physMin), physicalMax(physMax){}

    void set_filter(Filter* filter) {
        currentFilter = filter;
    }

    void set_min_max_values(int16_t minVal, int16_t maxVal) {
        minValue = minVal;
        maxValue = maxVal;
    }

    void set_physical_min_max(float physMin, float physMax) {
        physicalMin = physMin;
        physicalMax = physMax;
    }

    void add_value(int16_t value) {
        currentFilter->add_value(value);
    }

    int16_t get_filtered_value() {
        return currentFilter->get_filtered_value();
    }

    int16_t get_scaled_value() {
        int16_t filteredValue = get_filtered_value();
        return map(filteredValue, 0, 4095, minValue, maxValue);
    }

    float get_percentage_value() {
        float percentage = 100.0 / (maxValue - minValue) * (max(minValue, get_filtered_value()) - minValue);
        return percentage < 0.0 ? 0.0 : (percentage > 100.0 ? 100.0 : percentage);
    }

    float get_physical_value() {
        float filteredValue = get_filtered_value();
        filteredValue = filteredValue > maxValue ? maxValue : (filteredValue < minValue ? minValue : filteredValue);

        return physicalMin + ((physicalMax - physicalMin) / ((float)maxValue - (float)minValue) * (filteredValue - (float)minValue));
    }

    long map(long x, long in_min, long in_max, long out_min, long out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }   
};

#endif