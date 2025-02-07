#include "./mem_i2c_fm24gl64.h"

void FM24GL64::fill(unsigned int addr, const unsigned char& filler, size_t size)
{
    itcw->beginTransmission(this->device_i2c_addr);
    itcw->write((addr >> 8) & 0xFF);
    itcw->write(addr & 0xFF);
    for (size_t i = 0; i < size; ++i)
        itcw->write(filler);
    itcw->endTransmission();
}

bool FM24GL64::read(unsigned int addr, unsigned char* data, size_t size, bool check_last_short_crc)
{
    itcw->beginTransmission(this->device_i2c_addr);
    itcw->write((addr >> 8) & 0xFF);
    itcw->write(addr & 0xFF);
    itcw->endTransmission();

    itcw->requestFrom(this->device_i2c_addr, size);
    for (size_t i = 0; i < size; ++i)
        if (itcw->available())
            *(data + i) = itcw->read();

    return false;
}

void FM24GL64::write(unsigned int addr, const unsigned char* data, size_t size)
{
    itcw->beginTransmission(this->device_i2c_addr);
    itcw->write((addr >> 8) & 0xFF);
    itcw->write(addr & 0xFF);
    for (size_t i = 0; i < size; ++i)
        itcw->write(*(data + i));
    itcw->endTransmission();
}

char FM24GL64::byte_read(unsigned int addr)
{
    unsigned char byte = 0;
    this->read(addr, &byte, 1);
    return byte;
}

void FM24GL64::set_i2c_ch(I2C_CH* _itcw)
{
    this->itcw = _itcw;
}