#pragma once
#ifndef X_VAR_MVC_LVGL_V9
#define X_VAR_MVC_LVGL_V9

//#define DEV_USE_X_VAR_MVC_LVGL_V9

#ifdef DEV_USE_X_VAR_MVC_LVGL_V9
    #include "lvgl.h"
#else
    struct lv_subject_t {};
    inline void lv_subject_init_int(lv_subject_t*, int32_t) {}
    inline void lv_subject_set_int(lv_subject_t*, int32_t) {}
    inline void lv_subject_init_string(lv_subject_t*, char* buf, char* prev_buf, size_t size, const char* value) {}
    inline void lv_subject_copy_string(lv_subject_t*, const char* buf) {}
#endif // DEV_USE_X_VAR_MVC_LVGL_V9

#include <string>

class XVarMVC_LVGL_V9
{
private:
    lv_subject_t*   _lv_subject                 = nullptr;
    unsigned char   _lv_subject_str_buf_size    = 0;
    char*           _lv_subject_buf             = nullptr;
    char*           _lv_subject_prev_buf        = nullptr;

protected:
    virtual void update_lv_subject()
    {}

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

    lv_subject_t* get_lv_observable()
    {
        return _lv_subject;
    }
};

#endif
