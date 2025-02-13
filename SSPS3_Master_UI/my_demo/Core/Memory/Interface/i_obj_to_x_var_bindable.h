#pragma once
#ifndef I_OBJ_TO_X_VAR_BINDABLE_H
#define I_OBJ_TO_X_VAR_BINDABLE_H

#include <any>

class IObjToXVarBindable
{
public:
    IObjToXVarBindable() {}
    virtual ~IObjToXVarBindable() = default;

    virtual void on_x_var_value_changed(const std::any& newValue) = 0;

    virtual IObjToXVarBindable* get_observer() { return this; }

    template<typename T>
    T try_cast(const std::any& newValue)
    {
        try
        {
            return std::any_cast<T>(newValue);
        }
        catch (const std::bad_any_cast& e)
        {
            return T{};
        }
    }
};

#endif // !I_OBJ_TO_X_VAR_BINDABLE_H
