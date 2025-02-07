#pragma once
#ifndef X_VAR_T_STRUCT_H
#define X_VAR_T_STRUCT_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "../Data/Interface/i_struct_to_string.h"
    #include "./x_var_T.h"
#else
    #include "x_var_T.h"
    #include "i_struct_to_string.h"
#endif

template <typename T>
class XVar<T, std::enable_if_t<std::is_class_v<T>>> : public XVarBase<T>
{
protected:
    virtual std::string _get_str_from_x_var_t() override
    {
        if constexpr (std::is_base_of<IStructToString, T>::value)
        {
            IStructToString& _base_ref = this->_value;
            return _base_ref.to_string();
        }
        else
            return "<STRUCT>";
    }

    void update_lv_subject() override
    {
        XVarMVC::update_lv_subject_string(this->_get_str_from_x_var_t());
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

        XVarMVC::init_lv_subject_buffer(50);
        XVarMVC::init_lv_subject_string(this->_get_str_from_x_var_t());
    }

    XVar& operator=(const T& other)
    {
        this->_set_new_value_and_notify_subs(other);
        return *this;
    }

    bool operator==(const T& other) const { return this->_value == other; }
    bool operator!=(const T& other) const { return this->_value != other; }
    bool operator<(const T& other) const { return this->_value < other; }
    bool operator>(const T& other) const { return this->_value > other; }
    bool operator<=(const T& other) const { return this->_value <= other; }
    bool operator>=(const T& other) const { return this->_value >= other; }
};

#endif // !MEM_VAR_T_STRUCT_H
