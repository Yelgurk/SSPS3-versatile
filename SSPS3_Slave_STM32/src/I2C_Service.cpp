#include "../include/I2C_Service.hpp"

I2C_Service::I2C_Service(
    TwoWire * i2c,
    uint8_t addr,
    function<void(I2C_COMM, uint8_t, uint16_t)> set_event,
    function<uint16_t(I2C_COMM, uint8_t)> get_event,
    function<void(I2C_COMM)> finally_event
    )
{
    this->addr = addr;
    this->i2c = i2c;

    this->i2c->begin(this->addr);
    this->i2c->setClock(400000);

    this->set_event = set_event;
    this->get_event = get_event;
    this->finally_event = finally_event;

    this->i2c->onReceive([this](uint32_t)
    {
        if (this->i2c->available())
        {
            this->command     = static_cast<I2C_COMM>(this->i2c->read());
            this->pin   = this->i2c->read();
            this->val   = ((uint16_t)this->i2c->read() << 8) | this->i2c->read();

            if (this->command > I2C_COMM::SET_BEGIN && this->command < I2C_COMM::SET_END)
                this->set_event(this->command, this->pin, this->val);
            else
            if (this->command > I2C_COMM::GET_BEGIN && this->command < I2C_COMM::GET_END)
                this->response = this->get_event(this->command, this->pin);
            else
            if (this->command == I2C_COMM::STATE_PING)
                this->response = static_cast<uint16_t>(I2C_COMM::STATE_OK);

            this->finally_event(this->command);
        }
    });

    this->i2c->onRequest([this]()
    {
        this->i2c->write(highByte(this->response));
        this->i2c->write(lowByte(this->response));

        this->finally_event(this->command);
    });
}