#pragma once
#ifndef FILTER_EXPONENTIAL_SAMPLING_AVERAGE_H
#define FILTER_EXPONENTIAL_SAMPLING_AVERAGE_H

#include "../Interface/i_value_T_filter.h"
#include "../filter_exponential_smoothing.h"
#include "../filter_sampling_average.h"

template<typename T, typename X = T, unsigned char Size = 12, unsigned char Alpha = 6, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class FilterExponentialSamplingAverage : public IValueTFilter<T, X>
{
private:
    IValueTFilter<T, X>& _f_smlpng_avg = FilterSamplingAverage<T, X, Size>().get_i_value_filter();
    IValueTFilter<T, X>& _f_exp_smooth = FilterExponentialSmoothing<T, X>(static_cast<float>(Alpha) / 100.f).get_i_value_filter();

public:
    FilterExponentialSamplingAverage ()
    {}

    void add_value(T value) override
    {
        _f_smlpng_avg.add_value(value);

        if (_f_smlpng_avg.get_value_filtered_changed())
            _f_exp_smooth.add_value(_f_smlpng_avg.get_value_filtered());
    }
    
    T get_value_filtered() override {
        return _f_exp_smooth.get_value_filtered();
    }
};

#endif // !FILTER_EXPONENTIAL_SAMPLING_AVERAGE_H