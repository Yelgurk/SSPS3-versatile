#pragma once
#ifndef X_VAR_MVC_STRING_H
#define X_VAR_MVC_STRING_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <vector>
    #include <cstring>
    #include <string>
    #include <functional>
    #include "./XVarMVCExtension/x_var_mvc_lvgl_v9.h"
#else
    #include <vector>
    #include <cstring>
    #include <string>
    #include <functional>
    #include "x_var_mvc_lvgl_v9.h"
#endif

class XVarMVC_String
{
private:
    static std::function<void(void*, const char*)> on_x_var_changed_event_handler;
    std::vector<void*> _string_representation_ui_subscribers;

protected:
    void _mvc_string_ui_notify_x_var_changed(std::string value_str, bool only_last_sub = false)
    {
        if (!on_x_var_changed_event_handler)
            return;

        if (only_last_sub && !_string_representation_ui_subscribers.empty())
            on_x_var_changed_event_handler(_string_representation_ui_subscribers.back(), value_str.c_str());

        if (!only_last_sub)
            for (void* subscriber : _string_representation_ui_subscribers)
                on_x_var_changed_event_handler(subscriber, value_str.c_str());
    }

public:
    XVarMVC_String() {}

    static void mvc_string_binding_add_general_handler(std::function<void(void*, const char*)> _event)
    {
        XVarMVC_String::on_x_var_changed_event_handler = _event;
    }

    void virtual mvc_string_bind(void* ui_label_ptr)
    {
        _string_representation_ui_subscribers.push_back(ui_label_ptr);
    }
};

std::function<void(void*, const char*)> XVarMVC_String::on_x_var_changed_event_handler = nullptr;

#endif // !X_VAR_MVC_STRING_H
