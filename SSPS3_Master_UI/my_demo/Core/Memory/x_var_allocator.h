#pragma once
#ifndef X_VAR_ALLOCATOR_H
#define X_VAR_ALLOCATOR_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <any>
    #include <memory>
    #include <vector>
    #include "./x_var_T_arithmetic.h"
    #include "./x_var_T_string.h"
    #include "./x_var_T_struct.h"
#else
    #include <any>
    #include <memory>
    #include <vector>
    #include "x_var_T_arithmetic.h"
    #include "x_var_T_string.h"
    #include "x_var_T_struct.h"
#endif

using XVarVector = std::vector<std::unique_ptr<XVarFram>>;

template<typename T, typename... Args>
std::unique_ptr<T> custom_unique_init(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class XVarAllocator
{
private:
    XVarAllocator() {}

    XVarVector _external_mem_vars;
    unsigned int _free_addr = 0;

    void reset_all_ext_mem_var(bool system_val_reset = false, bool admin_val_reset = false, bool full_hard_reset = false)
    {
        for (auto& _ext_mem_var_unique_ptr : _external_mem_vars)
            _ext_mem_var_unique_ptr.get()->default_value_reset(
                system_val_reset,
                admin_val_reset,
                full_hard_reset
                );
    }

public:
    static XVarAllocator* instance()
    {
        static XVarAllocator inst;
        return &inst;
    }

    template<typename T>
    XVar<T>& allocate(T default_value, signed int ext_mem_var_addr, bool is_system_val, bool is_admin_val)
    {        
        _free_addr = ext_mem_var_addr > _free_addr ? ext_mem_var_addr : _free_addr;

        auto _unique_ptr = custom_unique_init<XVar<T>>(
            default_value,
            ext_mem_var_addr < 0 ? -1 : _free_addr,
            is_system_val,
            is_admin_val
        );

        XVar<T>* _ptr = _unique_ptr.get();
        _external_mem_vars.push_back(std::move(_unique_ptr));

        if (!_ptr->get_is_local_val())
            _free_addr += _ptr->get_value_with_crc_size();

        return *_ptr;
    }

    void device_first_boot()
    {
        vars_full_hard_reset();
    }

    void vars_user_reset()          { reset_all_ext_mem_var(); }
    void vars_sys_reset()           { reset_all_ext_mem_var(1); }
    void vars_admin_reset()         { reset_all_ext_mem_var(0, 1); }
    void vars_full_hard_reset()     { reset_all_ext_mem_var(0, 0, 1); }
};

#endif
