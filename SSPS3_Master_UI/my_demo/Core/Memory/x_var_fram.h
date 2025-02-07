#pragma once
#ifndef X_VAR_FRAM_H
#define X_VAR_FRAM_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./External/ext_mem_defines.h"
    #include "./x_var_extension.h"
#else
    #include "ext_mem_defines.h"
    #include "x_var_extension.h"
#endif

class XVarFram
{
private:
    bool            _is_system_val = false,
                    _is_admin_val = false,
                    _is_local_val = false;
    signed int      _ext_mem_address;
    unsigned short  _t_value_size,
                    _t_value_with_crc_size;
    unsigned char*  _t_value_with_crc_buffer;

protected:
    unsigned char*  _buffer_ptr()               { return _is_local_val ? NULL : _t_value_with_crc_buffer; }
    unsigned char*  _buffer_crc_ptr()           { return _is_local_val ? NULL : _buffer_ptr() + _t_value_with_crc_size - 1; }
    unsigned char   _calc_current_value_crc()   { return _is_local_val ? EXT_MEM_BYTE_ERR : CALC_CRC(_buffer_ptr(), get_value_size()); }

    void            _push_crc(unsigned char crc)
    {
        if (!_is_local_val)
            *_buffer_crc_ptr() = crc;
    }

public:
    XVarFram(signed int ext_mem_address, unsigned short t_value_size, bool is_system_val, bool is_admin_val) :
        _ext_mem_address(ext_mem_address),
        _t_value_size(t_value_size),
        _t_value_with_crc_size(t_value_size + 1),
        _t_value_with_crc_buffer(ext_mem_address < 0 ? NULL : new unsigned char[t_value_size + 1] {}),
        _is_system_val(is_system_val),
        _is_admin_val(is_admin_val),
        _is_local_val(ext_mem_address < 0)
    {

    }

    virtual ~XVarFram()
    {
        if (_t_value_with_crc_buffer != NULL)
            delete[] _t_value_with_crc_buffer;
    }

    virtual void            load_value_into_ui(bool only_last_subscriber_notify = false) = 0;
    virtual void            load_from_ext_mem() = 0;
    virtual void            default_value_reset(bool system_val_reset = false, bool admin_val_reset = false, bool full_hard_reset = false) = 0;
    virtual void            clear_ext_mem_area(unsigned char filler = 0x00) = 0;

    signed int      get_address()               { return _is_local_val ? -1 : _ext_mem_address; }
    signed int      get_crc_address()           { return _is_local_val ? -1 : get_address() + _t_value_with_crc_size - 1; }
    unsigned        get_value_size()            { return _t_value_size; }
    unsigned        get_value_with_crc_size()   { return _t_value_with_crc_size; }
    unsigned char   get_crc_from_buffer()       { return *_buffer_crc_ptr(); }
    bool            get_crc_match_ext_mem()     { return _is_local_val ? false : get_crc_from_buffer() == _calc_current_value_crc(); }
    bool            get_is_system_val()         { return _is_system_val; }
    bool            get_is_admin_val()          { return _is_admin_val; }
    bool            get_is_local_val()          { return _is_local_val; }
};

#endif // !MEM_VAR_BASE_H
