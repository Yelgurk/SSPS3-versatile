#pragma once
#ifndef X_VAR_MVC_OBJ_H
#define X_VAR_MVC_OBJ_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <vector>
    #include "./Interface/i_obj_to_x_var_bindable.h"
#else
    #include <vector>
    #include "i_obj_to_x_var_bindable.h"
#endif // DEV_SSPS3_RUN_ON_PLC

class XVarMVC_Obj
{
private:
    std::vector<IObjToXVarBindable*> _observers;

protected:
    void _mvc_obj_bindings_notify_value_changed(const std::any& newValue)
    {
        for (auto* observer : _observers)
        {
            if (observer)
                observer->on_x_var_value_changed(newValue);
        }
    }

public:
    void _mvc_obj_bind(IObjToXVarBindable* observer)
    {
        _observers.push_back(observer);
    }
};

#endif // !X_VAR_MVC_OBJ_H
