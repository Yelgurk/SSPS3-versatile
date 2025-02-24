#pragma once
#ifndef I_VALUE_SELECTION_FILTER_H
#define I_VALUE_SELECTION_FILTER_H

#include <Arduino.h>
#include <functional>

template<typename T, typename X = T, typename = std::enable_if_t<std::is_default_constructible<X>::value>>
class IValueTFilter
{
private:
    std::function<X(T)> _value_converter;

public:
    virtual void add_value(T value) = 0;

    virtual T get_value_filtered() = 0;

    virtual bool get_value_filtered_changed() { return true; }

    void add_converter(std::function<X(T)> value_converter) {
        this->_value_converter = value_converter;
    }

    //template<typename U = X, typename = std::enable_if_t<std::is_default_constructible<U>::value>>
    X get_value_converted()
    {
        if (_value_converter)
            return _value_converter(get_value_filtered());
        return X {};
    }

    IValueTFilter<T, X>& get_i_value_filter() {
        return *this;
    }
};

#endif // !I_VALUE_SELECTION_FILTER_H