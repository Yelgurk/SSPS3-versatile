#pragma once
#ifndef X_VAR_T_ARITHMETIC_H
#define X_VAR_T_ARITHMETIC_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_T.h"
#else
    #include "x_var_T.h"
#endif

template<typename T>
class XVar<T, std::enable_if_t<!std::is_class_v<T>>> : public XVarBase<T>
{
protected:
    virtual std::string _get_str_from_x_var_t() override
    {
        return std::to_string(this->_value);
    }

    void update_lv_subject() override
    {
        XVarMVC::update_lv_subject_int(static_cast<int32_t>(this->_value));
    }

public:
    XVar(T default_value, signed int ext_mem_address = -1, bool is_system_val = false, bool is_admin_val = false) :
        XVarBase<T>(
            default_value,
            ext_mem_address,
            sizeof(T),
            is_system_val,
            is_admin_val
        )
    {
        this->load_from_ext_mem();

        XVarMVC::init_lv_subject_int(static_cast<int32_t>(this->_value));
    }

    T operator+ (const XVar& other) { return this->_value + other._value; }
    T operator- (const XVar& other) { return this->_value - other._value; }
    T operator* (const XVar& other) { return this->_value * other._value; }
    T operator/ (const XVar& other) { return this->_value / other._value; }
    T operator% (const XVar& other) { return this->_value % other._value; }

    T operator++ () { this->_set_new_value_and_notify_subs(this->_value + 1); return this->_value; }
    T operator-- () { this->_set_new_value_and_notify_subs(this->_value - 1); return this->_value; }
    T operator++ (int) { this->_set_new_value_and_notify_subs(this->_value + 1); return this->_value - 1; }
    T operator-- (int) { this->_set_new_value_and_notify_subs(this->_value - 1); return this->_value + 1; }

    bool operator==(const XVar& other) { return this->_value == other._value; }
    bool operator!=(const XVar& other) { return this->_value != other._value; }
    bool operator<(const XVar& other) { return this->_value < other._value; }
    bool operator>(const XVar& other) { return this->_value > other._value; }
    bool operator<=(const XVar& other) { return this->_value <= other._value; }
    bool operator>=(const XVar& other) { return this->_value >= other._value; }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    T operator+ (const U& other) { return this->_value + static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    T operator- (const U& other) { return this->_value - static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    T operator* (const U& other) { return this->_value * static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    T operator/ (const U& other) { return this->_value / static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    T operator% (const U& other) { return this->_value % static_cast<T>(other); }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    XVar& operator=(const U& other) { this->_set_new_value_and_notify_subs(static_cast<T>(other)); return *this; }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    XVar& operator+=(const U& other) { this->_set_new_value_and_notify_subs(this->_value + static_cast<T>(other)); return *this; }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    XVar& operator-=(const U& other) { this->_set_new_value_and_notify_subs(this->_value - static_cast<T>(other)); return *this; }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    XVar& operator*=(const U& other) { this->_set_new_value_and_notify_subs(this->_value * static_cast<T>(other)); return *this; }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    XVar& operator/=(const U& other) { this->_set_new_value_and_notify_subs(this->_value / static_cast<T>(other)); return *this; }


    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator==(const U& other) { return this->_value == static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator!=(const U& other) { return this->_value != static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator<(const U& other) { return this->_value < static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator>(const U& other) { return this->_value > static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator<=(const U& other) { return this->_value <= static_cast<T>(other); }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    bool operator>=(const U& other) { return this->_value >= static_cast<T>(other); }
};

#endif // !X_VAR_T_ARITHMETIC_H
