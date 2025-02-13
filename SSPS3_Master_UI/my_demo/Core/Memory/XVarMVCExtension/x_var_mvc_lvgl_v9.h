#pragma once
#ifndef X_VAR_MVC_LVGL_V9
#define X_VAR_MVC_LVGL_V9

#define DEV_USE_X_VAR_MVC_LVGL_V9

#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
    #include "lvgl.h"
#else
    struct lv_obj_t {};
    struct lv_subject_t {};
    struct lv_observer_t {};
    inline void lv_subject_init_int(lv_subject_t*, int32_t) {}
    inline void lv_subject_set_int(lv_subject_t*, int32_t) {}
    inline void lv_subject_init_string(lv_subject_t*, char* buf, char* prev_buf, size_t size, const char* value) {}
    inline void lv_subject_copy_string(lv_subject_t*, const char* buf) {}
#endif // DEV_USE_X_VAR_MVC_LVGL_V9

#include <string>

typedef lv_observer_t* (*mvc_lvgl_bind_value_delegate)(lv_obj_t* observer, lv_subject_t* subject);
typedef lv_observer_t* (*mvc_lvgl_bind_text_delegate)(lv_obj_t* label, lv_subject_t* subject, const char* format);

class XVarMVC_LVGL_V9
{
private:
    lv_subject_t*   _lv_subject                 = nullptr;
    unsigned char   _lv_subject_str_buf_size    = 0;
    char*           _lv_subject_buf             = nullptr;
    char*           _lv_subject_prev_buf        = nullptr;

    static void _mvc_lvgl_observer_cb(lv_observer_t* observer, lv_subject_t* subject)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        if (observer && subject)
            static_cast<XVarMVC_LVGL_V9*>(lv_observer_get_user_data(observer))
                ->_mvc_lvgl_x_var_notify_ui_changed();
#endif
    }

protected:
    virtual void _mvc_lvgl_x_var_notify_value_changed()
    {}

    virtual void _mvc_lvgl_x_var_notify_ui_changed()
    {}

    lv_subject_t* get_lv_observable()
    {
        return _lv_subject;
    }

    void mvc_lvgl_bind_one_way_to_source(void* _lv_obj)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        lv_subject_add_observer_with_target
        (
            get_lv_observable(),
            _mvc_lvgl_observer_cb,
            _lv_obj,
            this
        );
#endif
    }

    void init_lv_subject_buffer(unsigned char buffer_size)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        _lv_subject_str_buf_size = buffer_size;
        _lv_subject_buf = new char[buffer_size]();
        _lv_subject_prev_buf = new char[buffer_size]();
#endif
    }

    void init_lv_subject_int(int32_t value)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        _lv_subject = new lv_subject_t;
        lv_subject_init_int(_lv_subject, value);
#else
        _lv_subject = nullptr;
#endif
    }

    void init_lv_subject_string(std::string value)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        _lv_subject = new lv_subject_t;
        lv_subject_init_string(_lv_subject, _lv_subject_buf, _lv_subject_prev_buf, _lv_subject_str_buf_size, value.c_str());
#else
        _lv_subject = nullptr;
#endif
    }

    void update_lv_subject_int(int32_t new_value)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        if (_lv_subject)
            lv_subject_set_int(_lv_subject, new_value);
#endif
    }

    void update_lv_subject_string(std::string new_value)
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        if (_lv_subject)
            lv_subject_copy_string(_lv_subject, new_value.c_str());
#endif
    }

public:
    XVarMVC_LVGL_V9()
    {}

    virtual ~XVarMVC_LVGL_V9()
    {
#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
        if (_lv_subject)
        {
            delete _lv_subject;
            _lv_subject = nullptr;
        }

        if (_lv_subject_buf)
            delete[] _lv_subject_buf;

        if (_lv_subject_prev_buf)
            delete[] _lv_subject_prev_buf;
#endif
    }

    lv_observer_t* mvc_lvgl_bind_one_way(lv_obj_t* bindable_ui_element, mvc_lvgl_bind_value_delegate lv_bind_method)
    {
        return lv_bind_method(bindable_ui_element, get_lv_observable());
    }

    lv_observer_t* mvc_lvgl_bind_one_way(lv_obj_t* bindable_label, mvc_lvgl_bind_text_delegate lv_bind_method, const char* format)
    {
        return lv_bind_method(bindable_label, get_lv_observable(), format);
    }

    lv_observer_t* mvc_lvgl_bind_two_way(lv_obj_t* bindable_ui_element, mvc_lvgl_bind_value_delegate lv_bind_method)
    {
        mvc_lvgl_bind_one_way_to_source(bindable_ui_element);
        return mvc_lvgl_bind_one_way(bindable_ui_element, lv_bind_method);
    }
};

#endif
