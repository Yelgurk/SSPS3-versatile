#pragma once
#ifndef FILTER_SAMPLING_AVERAGE_H
#define FILTER_SAMPLING_AVERAGE_H

#include "./Interface/i_value_T_filter.h"
#include "../../MCUsCommunication/Array/array_ring_buffer.h"

template<typename T, typename X = T, unsigned char Size = 12, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class FilterSamplingAverage : public IValueTFilter<T, X>
{
private:
    ArrayRingBuffer<T, Size> array;
    float prev_sampling_value = 0;

    bool is_inited          = false;
    bool is_value_changed   = false;

public:
    FilterSamplingAverage()
    {}

    void add_value(T value) override
    {
        if (!is_inited)
        {
            prev_sampling_value = static_cast<float>(value);
            is_value_changed = true;
            is_inited = true;
        }

        if (array.count() < Size)
            array.push(value);
        else
        {
            std::sort(array.get_array(), array.get_array() + array.count());
            
            prev_sampling_value = 0;
            for (unsigned char index = Size / 3; index < Size / 3 * 2; index++)
                prev_sampling_value += array[index];

            prev_sampling_value /= (Size / 3);

            is_value_changed = true;
            array.clear();
        }
    }
    
    T get_value_filtered() override
    {
        is_value_changed = false;
        return static_cast<T>(prev_sampling_value);
    }

    bool get_value_filtered_changed() override {
        return is_value_changed;
    }
};

#endif //! FILTER_MOVING_AVERAGE_H