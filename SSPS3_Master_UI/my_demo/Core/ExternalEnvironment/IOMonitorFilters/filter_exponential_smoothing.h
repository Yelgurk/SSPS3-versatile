#pragma once
#ifndef FILTER_EXPONENTIAL_SMOOTHING_H
#define FILTER_EXPONENTIAL_SMOOTHING_H

#include "./Interface/i_value_T_filter.h"

template<typename T, typename X = T, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class FilterExponentialSmoothing : public IValueTFilter<T, X>
{
private:
    float alpha;
    T filtered_value;
    bool is_initialized;
    float multiplier;

public:
    //template<typename U = T, typename = std::enable_if_t<std::is_default_constructible<U>::value>>
    FilterExponentialSmoothing(float alpha)
    :   alpha(alpha),
        filtered_value(T{}),
        is_initialized(false),
        multiplier(0.99f)
    {}

    void add_value(T value) override
    {
        filtered_value = is_initialized
            ? alpha * value + (1 - alpha) * filtered_value
            : value * multiplier;

        is_initialized = true;
    }

    T get_value_filtered() override {
        return filtered_value;
    }
};

#endif //! FILTER_EXPONENTIAL_SMOOTHING_H