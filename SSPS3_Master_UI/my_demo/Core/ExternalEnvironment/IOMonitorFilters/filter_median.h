#pragma once
#ifndef FILTER_MEDIAN_H
#define FILTER_MEDIAN_H

#include "./Interface/i_value_T_filter.h"
#include "../../MCUsCommunication/Array/array_ring_buffer.h"

template<typename T, typename X = T, unsigned char Size = 10, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class FilterMedian : public IValueTFilter<T, X>
{
private:
    ArrayRingBuffer<T, Size> array;
    T sorted_array[Size] = { T{} };

public:
    FilterMedian()
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

        memcpy(sorted_array, array.get_array(), array.count() * sizeof(T));
        std::sort(std::begin(sorted_array), std::end(sorted_array));
        
        return sorted_array[array.count() / 2];
    }
};

#endif //! FILTER_MEDIAN_H