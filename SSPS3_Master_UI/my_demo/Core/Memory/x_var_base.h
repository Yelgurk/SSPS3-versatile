#pragma once
#ifndef X_VAR_BASE_H
#define X_VAR_BASE_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./External/ext_mem_dispatcher.h"
    #include "./x_var_extension.h"
    #include "./x_var_fram.h"
    #include "./x_var_mvc.h"
#else
    #include "ext_mem_dispatcher.h"
    #include "x_var_extension.h"
    #include "x_var_fram.h"
    #include "x_var_mvc.h"
#endif

template<typename T>
class XVarBase : public XVarFram, public XVarMVC
{
protected:
    T   _value,
        _default_value;

    virtual std::string _get_str_from_x_var_t()
    {
        return "<NAN>";
    }

    //virtual void _push_value_into_buffer() = 0;
    //virtual void _pull_value_from_buffer() = 0;

    virtual void _push_value_into_buffer()
    {
        memcpy(
            XVarFram::_buffer_ptr(),
            &this->_value,
            XVarFram::get_value_size()
        );
    }

    virtual void _pull_value_from_buffer()
    {
        memcpy(
            &this->_value,
            XVarFram::_buffer_ptr(),
            XVarFram::get_value_size()
        );
    }

    void _set_new_value_and_notify_subs(T new_value) //(T& new_value) <- ???
    {
        _value = new_value;

        if (!XVarFram::get_is_local_val())
        {
            _push_value_into_buffer();

            XVarFram::_push_crc(XVarFram::_calc_current_value_crc());

            XStorageDispatcher->write(
                XVarFram::get_address(),
                XVarFram::_buffer_ptr(),
                XVarFram::get_value_with_crc_size()
            );
        }

        load_value_into_ui();   
        XVarMVC::update_lv_subject();
    }

public:
    XVarBase(T default_value, signed int ext_mem_address, signed short ext_mem_size, bool is_system_val, bool is_admin_val) :
        XVarFram(
            ext_mem_address,
            ext_mem_size,
            is_system_val,
            is_admin_val
        ),
        _default_value(default_value),
        _value(default_value)
    {}

    void load_from_ext_mem() override
    {
        if (!XVarFram::get_is_local_val() &&
            ExtMemDevicesDispatcher::instance()->read(
                XVarFram::get_address(),
                XVarFram::_buffer_ptr(),
                XVarFram::get_value_with_crc_size()
            ))
        {
            _pull_value_from_buffer();
            load_value_into_ui();
            return;
        }

        //get_crc_match_ext_mem() can be used instead included CRC comparator in ExtMemDevicesDispatcher::instance()->read(...) which one return true/false if readed successfully

        default_value_reset(0, 0, 1);
    }

    virtual void load_value_into_ui(bool only_last_subscriber_notify = false) override
    {
        XVarMVC::_notify_ui_subscribers
        (
            _get_str_from_x_var_t(),
            only_last_subscriber_notify
        );
    }

    void default_value_reset(bool system_val_reset = false, bool admin_val_reset = false, bool full_hard_reset = false) override
    {
        if ((!XVarFram::get_is_admin_val() && !XVarFram::get_is_system_val() && !admin_val_reset && !system_val_reset) ||
            (XVarFram::get_is_admin_val() && admin_val_reset) ||
            (XVarFram::get_is_system_val() && system_val_reset) ||
            full_hard_reset)
            _set_new_value_and_notify_subs(_default_value);
    }

    void clear_ext_mem_area(unsigned char filler = 0x00) override
    {
        ExtMemDevicesDispatcher::instance()->fill(
            XVarFram::get_address(),
            filler,
            XVarFram::get_value_with_crc_size()
        );
    }

    void subscribe_ui_label(void* lv_label_ptr) override
    {
        XVarMVC::subscribe_ui_label(lv_label_ptr);
        load_value_into_ui(true);
    }

    void save_changes()     { _set_new_value_and_notify_subs(_value); }
    void set(T new_value)   { _set_new_value_and_notify_subs(new_value); }
    T& get()                { return _value; }

    operator T&()           { return _value; }
};

#endif // !MEM_VAR_H
