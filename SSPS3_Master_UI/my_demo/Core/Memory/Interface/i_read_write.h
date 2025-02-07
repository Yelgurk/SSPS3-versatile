#pragma once
#ifndef I_READ_WRITE_H
#define I_READ_WRITE_H

class IReadWrite
{
public:
    virtual void fill(unsigned int addr, const unsigned char& filler, size_t size) = 0;
    virtual bool read(unsigned int addr, unsigned char* data, size_t size, bool check_last_short_crc = false) = 0;
    virtual void write(unsigned int addr, const unsigned char* data, size_t size) = 0;
    virtual char byte_read(unsigned int addr) = 0;
    
    IReadWrite* get_i_read_write()
    { 
        return this;
    }
};

#endif // !I_READ_WRITE_H
