#pragma once
#ifndef FILTER_MOVING_AVERAGE_H
#define FILTER_MOVING_AVERAGE_H

#include "./Interface/i_value_T_filter.h"
#include "../../MCUsCommunication/Array/array_ring_buffer.h"

template<typename T, typename X = T, unsigned char Size = 10, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class FilterMovingAverage : public IValueTFilter<T, X>
{
private:
    ArrayRingBuffer<T, Size> array;

public:
    FilterMovingAverage()
    {}

    void add_value(T value) override
    {
        if (array.count() >= Size)
            array.pop();

        array.push(value);
    }
    
    T get_value_filtered() override
    {
        if (array.empty())
            return 0;

        float sum = 0;
        for (T value : array)
            sum += value;

        return static_cast<T>(sum / static_cast<float>(array.count));
    }
};

#endif //! FILTER_MOVING_AVERAGE_H