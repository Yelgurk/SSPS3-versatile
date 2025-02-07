#pragma once
#ifndef X_VAR_MVC_H
#define X_VAR_MVC_H

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

class XVarMVC : public XVarMVC_LVGL_V9
{
private:
    static std::function<void(void*, const char*)> _ui_label_notify_value_changed_event;
    std::vector<void*> _ui_label_subscribers;

protected:
    void _notify_ui_subscribers(std::string value_str, bool only_last_sub = false)
    {
        if (!_ui_label_notify_value_changed_event)
            return;

        if (only_last_sub && !_ui_label_subscribers.empty())
            _ui_label_notify_value_changed_event(_ui_label_subscribers.back(), value_str.c_str());

        if (!only_last_sub)
            for (void* subscriber : _ui_label_subscribers)
                _ui_label_notify_value_changed_event(subscriber, value_str.c_str());
    }

public:
    XVarMVC() {}

    static void set_ui_label_notify_event(std::function<void(void*, const char*)> _event)
    {
        XVarMVC::_ui_label_notify_value_changed_event = _event;
    }

    void virtual subscribe_ui_label(void* ui_label_ptr)
    {
        _ui_label_subscribers.push_back(ui_label_ptr);
    }
};

std::function<void(void*, const char*)> XVarMVC::_ui_label_notify_value_changed_event = nullptr;

#endif // !MEM_VAR_MVC_H
