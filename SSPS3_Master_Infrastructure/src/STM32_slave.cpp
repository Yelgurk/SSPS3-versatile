#include "../include/STM32_slave.hpp"

void STM32_slave::_set(I2C_COMM cmd, uint8_t pin, uint16_t val)
{
    if (!connection_error)
    {
        buf[0] = static_cast<uint8_t>(cmd);
        buf[1] = pin;
        buf[2] = highByte(val);
        buf[3] = lowByte(val);

        itcw->beginTransmission(addr);
        itcw->write((uint8_t *) buf, 4);
        itcw->endTransmission();

        delayMicroseconds(160);
    }
}
    
uint16_t STM32_slave::_get(I2C_COMM cmd, uint8_t pin)
{
    if (!connection_error)
    {
        _set(cmd, pin, 0);

        itcw->requestFrom(addr, (uint8_t)2);
        delayMicroseconds(40);

        if (itcw->available())
            response = ((uint16_t)itcw->read() << 8) | itcw->read();

        return response;
    }
    else
        return 0;
}

bool STM32_slave::ping()
{
    if (static_cast<I2C_COMM>(this->_get(I2C_COMM::STATE_PING, 0)) != I2C_COMM::STATE_OK)
        connection_error = true;
    else
        connection_error = false;

    return !connection_error;
}

void STM32_slave::startup()
{
    this->_set(I2C_COMM::STATE_STARTUP, 0, 0);
}

void STM32_slave::set(COMM_SET cmd, uint8_t pin, uint16_t val)
{    
    this->ping();
    this->_set(static_cast<I2C_COMM>(cmd), pin, val);
}

uint16_t STM32_slave::get(COMM_GET cmd, uint8_t pin)
{
    this->ping();
    return this->_get(static_cast<I2C_COMM>(cmd), pin);
}

uint16_t STM32_slave::get_kb()
{
    this->ping();
    return _get(I2C_COMM::GET_KB_VAL, 0);
}