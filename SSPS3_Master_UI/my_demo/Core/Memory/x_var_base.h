#pragma once
#ifndef X_VAR_BASE_H
#define X_VAR_BASE_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./External/ext_mem_dispatcher.h"
    #include "./x_var_extension.h"
    #include "./x_var_fram.h"
    #include "./x_var_mvc_string.h"
    #include "./x_var_mvc_obj.h"
#else
    #include "ext_mem_dispatcher.h"
    #include "x_var_extension.h"
    #include "x_var_fram.h"
    #include "x_var_mvc_string.h"
    #include "x_var_mvc_obj.h"
#endif

template<typename U, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename U>
struct is_equality_comparable<U, std::void_t<decltype(std::declval<U>() == std::declval<U>())>> : std::true_type {};

template<typename T>
class XVarBase :
    public XVarFram,
    public XVarMVC_String,
    public XVarMVC_LVGL_V9,
    public XVarMVC_Obj
{
protected:
    T   _value,
        _default_value;

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

    virtual std::string _get_str_from_x_var_t()
    {
        return "<NAN>";
    }

    void _set_new_value_and_notify_subs(T new_value, bool _mvc_target_one_way_to_source = false, bool _force_data_update_anyway = false)
    {
        if (!_force_data_update_anyway && _value == new_value)
            return;

        _value = new_value;

        if (!XVarFram::get_is_local_val())
        {
            _push_value_into_buffer();

            XVarFram::_push_crc(XVarFram::_calc_current_value_crc());

            ExtMemDevicesDispatcher::instance()->write(
                XVarFram::get_address(),
                XVarFram::_buffer_ptr(),
                XVarFram::get_value_with_crc_size()
            );
        }

        load_value_into_ui();
        _mvc_obj_bindings_notify_value_changed(std::any(_value));

        if (!_mvc_target_one_way_to_source)
            _mvc_lvgl_x_var_notify_value_changed();
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

    virtual void load_from_ext_mem() override
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

        default_value_reset(0, 0, 1);
    }

    virtual void load_value_into_ui(bool only_last_subscriber_notify = false) override
    {
        XVarMVC_String::_mvc_string_ui_notify_x_var_changed
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
            _set_new_value_and_notify_subs(_default_value, false, true);
    }

    void clear_ext_mem_area(unsigned char filler = 0x00) override
    {
        ExtMemDevicesDispatcher::instance()->fill(
            XVarFram::get_address(),
            filler,
            XVarFram::get_value_with_crc_size()
        );
    }

    void mvc_string_bind(void* lv_label_ptr) override
    {
        XVarMVC_String::mvc_string_bind(lv_label_ptr);
        load_value_into_ui(true);
    }

    void save_changes()     { _set_new_value_and_notify_subs(_value, false, true); }
    void set(T new_value)   { _set_new_value_and_notify_subs(new_value); }
    T& get()                { return _value; }
    T  get_default()        { return _default_value; }

    template<typename U = T>
    std::enable_if_t<is_equality_comparable<U>::value, bool>
    is_equal_to_default() const {
        return _value == _default_value;
    }

    operator T&()           { return _value; }
};

#endif // !MEM_VAR_H
