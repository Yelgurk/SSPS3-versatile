#pragma once
#ifndef X_VAR_T_STRING_H
#define X_VAR_T_STRING_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./x_var_T.h"
#else
    #include "x_var_T.h"
#endif

template <>
class XVar<std::string> : public XVarBase<std::string>
{
protected:
    virtual void _push_value_into_buffer() override
    {
        size_t string_size = this->_value.size();
        size_t buffer_size = get_value_size();

        if (string_size >= buffer_size)
            string_size = buffer_size - 1; // -1 => space for end-string symbol

        memcpy(
            XVarFram::_buffer_ptr(),
            this->_value.c_str(),
            string_size
        );

        XVarFram::_buffer_ptr()[string_size] = '\0';

        _pull_value_from_buffer();
    }

    virtual void _pull_value_from_buffer() override
    {
        this->_value = std::string(reinterpret_cast<char*>(XVarFram::_buffer_ptr()));
    }

protected:
    // for XVarMVC_String
    virtual std::string _get_str_from_x_var_t() override
    {
        return this->_value;
    }

    // for XVarMVC_LVGL_V9
    void _mvc_lvgl_x_var_notify_value_changed() override
    {
        XVarMVC_LVGL_V9::update_lv_subject_string(this->_get_str_from_x_var_t());
    }

public:
    XVar(const std::string& default_value, signed int ext_mem_address = -1, bool is_system_val = false, bool is_admin_val = false) :
        XVarBase<std::string>(
            default_value,
            ext_mem_address,
            ext_mem_address < 0 ? 50 : 20, // if local val (addr < 0) => 50 char lenght, else if in i2c eeprom/fram external mem (addr >= 0) => 20 char lenght
            is_system_val,
            is_admin_val
        )
    {
        this->load_from_ext_mem();

        XVarMVC_LVGL_V9::init_lv_subject_buffer(this->get_value_size());
        XVarMVC_LVGL_V9::init_lv_subject_string(this->_get_str_from_x_var_t());
    }

    XVar& operator=(const std::string& other) { this->_set_new_value_and_notify_subs(other); return *this; }

    bool operator==(const std::string& other) const { return this->_value == other; }
    bool operator!=(const std::string& other) const { return this->_value != other; }
    bool operator<(const std::string& other) const { return this->_value < other; }
    bool operator>(const std::string& other) const { return this->_value > other; }
    bool operator<=(const std::string& other) const { return this->_value <= other; }
    bool operator>=(const std::string& other) const { return this->_value >= other; }
};

#endif // !MEM_VAR_T_STRING_H
