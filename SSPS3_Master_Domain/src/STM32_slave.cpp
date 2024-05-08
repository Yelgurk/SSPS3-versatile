#include "../include/STM32_slave.hpp"

STM32_slave::STM32_slave(TwoWire * i2c, uint8_t addr, uint8_t sda, uint8_t scl)
{
    this->addr = addr;
    this->i2c = i2c;
    this->i2c->begin(sda, scl, 400000);
}

void STM32_slave::_set(I2C_COMM cmd, uint8_t pin, uint16_t val)
{
    buf[0] = static_cast<uint8_t>(cmd);
    buf[1] = pin;
    buf[2] = highByte(val);
    buf[3] = lowByte(val);

    i2c->beginTransmission(addr);
    i2c->write((uint8_t *) buf, 4);
    i2c->endTransmission();

    delayMicroseconds(160);
}
    
uint16_t STM32_slave::_get(I2C_COMM cmd, uint8_t pin)
{
    _set(cmd, pin, 0);

    i2c->requestFrom(addr, (uint8_t)2);
    delayMicroseconds(40);

    if (i2c->available())
        response = ((uint16_t)i2c->read() << 8) | i2c->read();

    return response;
}

void STM32_slave::set(COMM_SET cmd, uint8_t pin, uint16_t val) {
    this->_set(static_cast<I2C_COMM>(cmd), pin, val);
}

uint16_t STM32_slave::get(COMM_GET cmd, uint8_t pin) {
    return this->_get(static_cast<I2C_COMM>(cmd), pin);
}

uint16_t STM32_slave::get_kb() {
    return _get(I2C_COMM::GET_KB_VAL, 0);
}