#pragma once
#ifndef MEM_DISPATCHER_H
#define MEM_DISPATCHER_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include <vector>
    #include <string>
    #include <string.h>
    #include "../Interface/i_read_write.h"
    #include "../x_var_extension.h"
    #include "./mem_defines.h"
#else
    #include <vector>
    #include "i_read_write.h"
    #include "x_var_extension.h"
    #include "mem_defines.h"
#endif

class MemDevicesDispatcher : public IReadWrite
{
private:
    MemDevicesDispatcher() {}
    std::vector<IReadWrite*>    mem_devices;
    std::vector<unsigned char>  mem_devices_crc;
    unsigned char*              _disp_buff = new unsigned char[MEM_READ_BUFF_SIZE]{};

public:
    ~MemDevicesDispatcher()
    {
        delete[] _disp_buff;
    }

    static MemDevicesDispatcher* instance()
    {
        static MemDevicesDispatcher inst;
        return &inst;
    }

    void add_device(IReadWrite* device)
    {
        mem_devices.push_back(device);
        mem_devices_crc.push_back(0);
    }

    void fill(unsigned int addr, const unsigned char& filler, size_t size) override
    {
        for (auto ext_mem : mem_devices)
            ext_mem->fill(addr, filler, size);
    }

    bool read(unsigned int addr, unsigned char* data, size_t size, bool check_last_short_crc = false) override // bool check_last_short_crc = false - impl, but not used
    {
        bool _readed_successfully = false;

        for (auto& ext_mem_crc : mem_devices_crc)
            ext_mem_crc = 0;

        for (unsigned char i = 0; i < mem_devices.size(); i++)
        {
            mem_devices.at(i)->read(addr, _disp_buff, size);
            mem_devices_crc.at(i) = CALC_CRC(_disp_buff, size - 1);

            if (mem_devices_crc.at(i) == *(_disp_buff + size - 1))
            {
                for (unsigned char err = 0; err < i; err++)
                    mem_devices.at(err)->write(addr, _disp_buff, size);

                _readed_successfully = true;
                break;
            }
        }

        if (_readed_successfully)
            memcpy(data, _disp_buff, size);

        return _readed_successfully;
    }

    void write(unsigned int addr, const unsigned char* data, size_t size) override
    {
        for (auto ext_mem : mem_devices)
            ext_mem->write(addr, data, size);
    }

    char byte_read(unsigned int addr) override
    {
        static unsigned char i = 0, byte = 0x00;

        i = 0;
        byte = mem_devices.at(i++)->byte_read(addr);

        while (i < mem_devices.size())
            if (mem_devices.at(i++)->byte_read(addr) != byte)
            {
                byte = MEM_BYTE_ERR;
                break;
            }

        return byte;
    }
};

#endif // !MEM_DISPATCHER_H
