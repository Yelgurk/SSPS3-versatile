#pragma once
#ifndef MY_MQTT_I2C_H
#define MY_MQTT_I2C_H

#include <Arduino.h>
#include "Wire.h"
#include <vector>
#include <queue>

enum MqttCommand : uint8_t
{
    CMD_NAN,
    GET_D_IO,
    GET_A_IN,
    GET_KB,
    SET_A_OUT,
    SET_R_OUT
};

#pragma pack(push, 1)
struct MqttMessage
{
    unsigned char command     = MqttCommand::CMD_NAN;
    unsigned char addr        = 0x02;
    unsigned char content[2]  = { 0x00 };

    MqttMessage()
    {}

    MqttMessage(unsigned char command, unsigned char addr)
    {
        this->command   = command;
        this->addr      = addr;
    }

    template<typename T>
    void fill_content(void* value_in)
    {
        memset(content, 0, sizeof(content))
        memcpy(content, value_in, sizeof(T));
    }

    template<typename T>
    void get_content(T* value_out)
    {
        memcpy(reinterpret_cast<unsigned char*>(value_out), content, sizeof(T));
    }

    MqttMessage* set_has_a_following_messages(bool valid)
    {
        if(valid)
            command |= 0b10000000;
        else
            command &= 0b01111111;

        return this;
    }

    bool get_has_a_following_messages() {
        return (command & 0b10000000) != 0;
    }

    MqttCommand get_command() {
        return static_cast<MqttCommand>(command & 0b01111111);
    }

    MqttMessage* set_addr(unsigned char addr)
    {
        this->addr = addr;
        return this;
    }

    static MqttMessage* get_dummy(unsigned char addr)
    {
        static MqttMessage dummy;
        return dummy.set_addr(addr);
    }

    static unsigned char get_size_of()
    {
        static unsigned char _size_of = sizeof(MqttMessage);
        return _size_of;
    } 
};
#pragma pack(pop)

typedef std::function<void(int)>            OnReceiveEvent;
typedef std::function<void()>               OnRequestEvent;
typedef std::function<void(MqttMessage)>    AfterReceiveEvent;

struct MqttSlaveSubscriber
{
    unsigned char   address;
    char            interrupt_pin;
    volatile bool   is_master_called = false;

    std::vector<std::pair<unsigned char, AfterReceiveEvent>> command_handlers;
    
    MqttSlaveSubscriber(unsigned char address, unsigned char interrupt_pin)
    {
        this->address           = address;
        this->interrupt_pin     = interrupt_pin;
        this->is_master_called  = false;
    }
};

class MyMqttI2C
{
private:
    MyMqttI2C() {}

    std::unordered_map<unsigned char, MqttSlaveSubscriber> _master_side_subscriptions;
    std::vector<std::pair<unsigned char, AfterReceiveEvent>> _slave_side_command_handlers;
    std::queue<MqttMessage> _mqtt_incoming_queue;
    std::queue<MqttMessage> _mqtt_outgoing_queue;

    TwoWire*    _i2c;
    char        _address = 0x01;
    bool        _is_master = false;
    char        _interrupt_pin = -1;

    static void IRAM_ATTR static_interrupt_wrapper()
    {
        noInterrupts();
        for(auto& sub : MyMqttI2C::instance()->_master_side_subscriptions)
        {
            // для attachInterrupt CHANGE
            sub.second.is_master_called = digitalRead(sub.second.interrupt_pin);

            // для attachInterrupt RISING
            //if (digitalRead(sub.second.interrupt_pin))
            //    sub.second.is_master_called = true;
        }
        interrupts();
    }

    static void static_on_receive(int numBytes)
    {
        static MyMqttI2C* inst = MyMqttI2C::instance();

        if  (inst->_i2c->available() >= MqttMessage::get_size_of() &&
            inst->_i2c->available() % MqttMessage::get_size_of() == 0)
        {
            MqttMessage incoming_message;

            while (numBytes > 0)
            {
                numBytes -= MqttMessage::get_size_of();

                inst->_i2c->readBytes(
                    reinterpret_cast<uint8_t*>(&incoming_message),
                    MqttMessage::get_size_of()
                );

                inst->_mqtt_incoming_queue.push(MqttMessage(incoming_message));
            }
        }
        else
        {
            while(inst->_i2c->available())
                inst->_i2c->read();
        }
    }

    static void static_on_request()
    {
        static MyMqttI2C* inst = MyMqttI2C::instance();
        short _in_queue = inst->_mqtt_outgoing_queue.size();

        if (_in_queue <= 0)
        {
            inst->_set_interrupt_signal(false);

            inst->_i2c->write(
                reinterpret_cast<uint8_t*>(MqttMessage::get_dummy(inst->_address)),
                MqttMessage::get_size_of()
            );
        }
        else
        {
            if (--_in_queue == 0)
                inst->_set_interrupt_signal(false);

            MqttMessage outgoing_message = inst->_mqtt_outgoing_queue.front();

            inst->_i2c->write(
                reinterpret_cast<uint8_t*>(outgoing_message.set_has_a_following_messages(_in_queue > 0)),
                MqttMessage::get_size_of()
            );
            
            inst->_mqtt_outgoing_queue.pop();
        }
    }

    void _set_interrupt_signal(bool state)
    {
        if (!this->_is_master)
            digitalWrite(this->_interrupt_pin, state);
    }

    bool _get_interrupt_signal(unsigned char address = 0x01)
    {
        if (!this->_is_master)
        {
            return digitalRead(this->_interrupt_pin);
        }
        else if (address > 0x01 && _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            return digitalRead(_master_side_subscriptions[address].interrupt_pin);
        }
        else if (!_master_side_subscriptions.empty())
        {
            for (auto& sub : _master_side_subscriptions)
                if (digitalRead(sub.second.interrupt_pin))
                {
                    return true;
                    break;
                }

            return false;
        }
        else
        {
            return false;
        }
    }

    void read_subscribers()
    {
        if (!this->_is_master)
            return;

        // проверяем, вызывает ли master-а какой из slave-ов
        static_interrupt_wrapper();
        // закомментировал, т.к. вроде достаточно самого attachInterrupt(...) в subscribe_slave(...)
        // по итогу на всякий случай раскомментировал

        for (auto& sub : _master_side_subscriptions)
            if (sub.second.is_master_called)
                while(sub.second.is_master_called = digitalRead(sub.second.interrupt_pin))
                {
                    _i2c->requestFrom(sub.second.address, MqttMessage::get_size_of());
                    delayMicroseconds(100);

                    if (_i2c->available())
                    {
                        MqttMessage incoming;

                        _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessage::get_size_of());
                        _mqtt_incoming_queue.push(incoming);

                        if (!incoming.get_has_a_following_messages())
                        {
                            sub.second.is_master_called = false;
                            break;
                        }
                    }
                }
    }

    void update_incoming()
    {
        if (this->_is_master)
        {
            // тут нужна логика, где каждое полученное сообщение, через цикл, master-ом из _mqtt_incoming_queue
            // будет по адресу сверяться и находиться нужный slave подписчик из _master_side_subscriptions[...]
            // а затем выбирается тот handler у MqttSlaveSubscriber-а, который совпадает с его парой-ключом относительно command,
            // которая в свою очередь берётся через get_command() метод у самого сообщения
        }
        else
        {
            // тут логика проще, т.к. у slave нет подпичсиков. тут просто все сообщения, независимо от адресата, "проганются"
            // через _slave_side_command_handlers очередь и если найден тот обработчик, который закрепён за командой - вызвать.
        }
    }

    void update_outgoing()
    {
        if (this->_is_master)
        {
            short _in_queue = _mqtt_outgoing_queue.size();
            while(_in_queue-- > 0)
            {
                MqttMessage _outgoing = _mqtt_outgoing_queue.front();

                _i2c->beginTransmission(_outgoing.addr);
                _i2c->write(
                    reinterpret_cast<uint8_t*>(&_outgoing),
                    MqttMessage::get_size_of()
                );
                _i2c->endTransmission();

                _mqtt_outgoing_queue.pop();

                delayMicroseconds(100);
            } 
        }
        else if (!_mqtt_outgoing_queue.empty())
        {
            _set_interrupt_signal(true);
        }
    }

public:
    static MyMqttI2C* instance()
    {
        static MyMqttI2C inst;
        return &inst;
    }

    TwoWire* begin(unsigned char sda, unsigned char scl, unsigned int freq, bool is_master, unsigned char addr, char interrupt_pin = -1)
    {
        if (this->_is_master = is_master)
        {
#ifdef DEV_SSPS3_IS_MASTER
            this->_i2c = new TwoWire(0);
            this->_i2c->begin(sda, scl, freq);      
#endif
        }
        else
        {
#ifdef DEV_SSPS3_IS_SLAVE
            this->_interrupt_pin = interrupt_pin;

            this->_i2c = new TwoWire(sda, scl);
            this->_i2c->setClock(freq);
            this->_i2C->begin(this->_address = addr <= 0x01 ? 0x02 : addr);

            if (_interrupt_pin != -1)
            {
                pinMode(_interrupt_pin, OUTPUT);
                digitalWrite(_interrupt_pin, LOW);
            }

            this->_i2c->onReceive(static_on_receive);
            this->_i2c->onRequest(static_on_request);
#endif
        }
    }

    bool set_after_receive_handler(unsigned char command, AfterReceiveEvent handler, unsigned char address = 0x01)
    {
        if (this->_is_master && _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            _master_side_subscriptions[address].command_handlers.push_back({ command, handler });
            return true;
        }
        else
        {
            _slave_side_command_handlers.push_back({ command, handler });
            return true;
        }

        return false;
    }

    void subscribe_slave(unsigned char address, unsigned char master_to_slave_interrupt_pin)
    {
        this->_master_side_subscriptions[address] = MqttSlaveSubscriber(address, master_to_slave_interrupt_pin);

        pinMode(
            _master_side_subscriptions[address].interrupt_pin,
            INPUT_PULLDOWN
        );

        //в прошлом был RISING
        attachInterrupt(
            digitalPinToInterrupt(_master_side_subscriptions[address].interrupt_pin),
            MyMqttI2C::instance()->static_interrupt_wrapper,
            CHANGE 
        );
    }

    bool push_message(MqttMessage new_message, unsigned char address = 0x01)
    {
        if (this->_is_master &&
            address > 0x01 &&
            _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            _mqtt_outgoing_queue.push(*new_message.set_addr(address));
            return true;
        }
        else if (!this->_is_master)
        {
            _mqtt_outgoing_queue.push(new_message);
            _set_interrupt_signal(true);

            return true;
        }

        return false;
    }

    void update()
    {
        read_subscribers();
        update_incoming();
        update_outgoing();
    }
};

#endif