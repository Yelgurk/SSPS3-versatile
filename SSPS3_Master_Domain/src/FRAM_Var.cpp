#include "../include/FRAM_Var.hpp"

void FRAM_write(uint32_t startAddress, void *data, uint32_t len)
{
    itcw->beginTransmission(FRAM_address);
    itcw->write((uint8_t*) &startAddress, 4);
    itcw->write((uint8_t*) data, len);
    itcw->endTransmission(true);

    delayMicroseconds(160);
}

uint32_t FRAM_read(uint32_t startAddress, void *data, uint32_t len)
{
    uint32_t readed;
    uint8_t *p;

    itcw->beginTransmission(FRAM_address);
    itcw->write((uint8_t*) &startAddress, 4);
    itcw->endTransmission();

    itcw->requestFrom(FRAM_address, len);
    for (readed = 0, p = (uint8_t*)data; itcw->available() && readed < len; readed++, p++)
        *p = itcw->read();

    return (readed);
}

uint8_t FRAM_readByte(uint32_t address)
{
    itcw->beginTransmission(FRAM_address);
    itcw->write((uint8_t*) &address, 4);
    itcw->endTransmission();

    itcw->requestFrom(FRAM_address, (uint8_t)1);
    if (itcw->available())
        return itcw->read();

    return 0;
}