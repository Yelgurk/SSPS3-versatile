#pragma once
#ifndef MY_MQTT_I2C_H
#define MY_MQTT_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstring> // для memcpy, memset
#include <functional>

// Определение команд MQTT
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
// Структура сообщения MQTT
struct MqttMessage
{
    uint8_t command = MqttCommand::CMD_NAN;  // Команда (с флагом наличия следующих сообщений в старшем бите)
    uint8_t addr    = 0x02;                  // Адрес получателя
    uint8_t content[2] = {0x00};             // Поле для данных

    MqttMessage() {}

    MqttMessage(uint8_t command, uint8_t addr)
    {
        this->command = command;
        this->addr = addr;
    }

    // Заполнение поля content данными, размер которых соответствует типу T
    template<typename T>
    void set_content(void* value_in)
    {
        memset(content, 0, sizeof(content)); // Обнуление буфера
        memcpy(content, value_in, sizeof(T));
    }

    // Извлечение данных из content в переменную типа T
    template<typename T>
    void get_content(T* value_out)
    {
        memcpy(reinterpret_cast<uint8_t*>(value_out), content, sizeof(T));
    }

    // Установка флага наличия следующих сообщений
    MqttMessage* set_has_a_following_messages(bool valid)
    {
        if (valid)
            command |= 0b10000000;
        else
            command &= 0b01111111;
        return this;
    }

    // Получение флага наличия следующих сообщений
    bool get_has_a_following_messages() const {
        return (command & 0b10000000) != 0;
    }

    // Получение команды без флага
    MqttCommand get_command() const {
        return static_cast<MqttCommand>(command & 0b01111111);
    }

    // Установка адреса получателя
    MqttMessage* set_addr(uint8_t addr)
    {
        this->addr = addr;
        return this;
    }

    // Возвращает указатель на dummy-сообщение с заданным адресом
    static MqttMessage* get_dummy(uint8_t addr)
    {
        static MqttMessage dummy;
        return dummy.set_addr(addr);
    }

    // Размер структуры сообщения
    static uint8_t get_size_of()
    {
        static uint8_t _size_of = sizeof(MqttMessage);
        return _size_of;
    } 
};
#pragma pack(pop)

// Определения типов обработчиков событий
typedef std::function<void(int)> OnReceiveEvent;
typedef std::function<void()> OnRequestEvent;
typedef std::function<void(MqttMessage)> AfterReceiveEvent;

// Структура подписчика слейва (на стороне мастера)
struct MqttSlaveSubscriber
{
    uint8_t address = 0x02;         // Адрес слейва
    char interrupt_pin = -1;      // Номер пина для прерываний
    volatile bool is_master_called = false;  // Флаг прерывания

    // Вектор пар: (команда, обработчик)
    std::vector<std::pair<uint8_t, AfterReceiveEvent>> command_handlers;
    
    MqttSlaveSubscriber()
    {}

    MqttSlaveSubscriber(uint8_t address, uint8_t interrupt_pin)
        : address(address), interrupt_pin(interrupt_pin), is_master_called(false) {}
};

class MyMqttI2C
{
private:
    // Приватный конструктор (синглтон)
    MyMqttI2C() {}

    // Подписки на стороне мастера: ключ – адрес слейва
    std::unordered_map<uint8_t, MqttSlaveSubscriber> _master_side_subscriptions;
    // Обработчики на стороне слейва
    std::vector<std::pair<uint8_t, AfterReceiveEvent>> _slave_side_command_handlers;
    // Очереди входящих и исходящих сообщений
    std::queue<MqttMessage> _mqtt_incoming_queue;
    std::queue<MqttMessage> _mqtt_outgoing_queue;

    TwoWire* _i2c = nullptr;      // Интерфейс I2C
    uint8_t _address = 0x01;      // Адрес устройства
    bool _is_master = false;      // Режим работы (мастер/слейв)
    char _interrupt_pin = -1;     // Пин для сигнализации (используется на стороне слейва)

    // Обработчик прерывания для мастера: проверяет состояние пинов от подписанных слейвов
    static void static_interrupt_wrapper()
    {
        noInterrupts();
        for(auto& sub_pair : MyMqttI2C::instance()->_master_side_subscriptions)
        {
            // Чтение состояния пина прерывания
            sub_pair.second.is_master_called = digitalRead(sub_pair.second.interrupt_pin);
        }
        interrupts();
    }

    // Колбэк обработки события onReceive для I2C
    static void static_on_receive(int numBytes)
    {
        MyMqttI2C* inst = MyMqttI2C::instance();

        // Если доступно целое число сообщений
        if (inst->_i2c->available() >= MqttMessage::get_size_of() &&
            inst->_i2c->available() % MqttMessage::get_size_of() == 0)
        {
            while (numBytes >= MqttMessage::get_size_of())
            {
                MqttMessage incoming_message;
                // Чтение сообщения по размеру структуры
                inst->_i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming_message), MqttMessage::get_size_of());
                noInterrupts();
                inst->_mqtt_incoming_queue.push(incoming_message);
                interrupts();
                numBytes -= MqttMessage::get_size_of();
            }
        }
        else
        {
            // Если размер данных некорректен – сброс буфера
            while(inst->_i2c->available())
                inst->_i2c->read();
        }
    }

    // Колбэк обработки события onRequest для I2C (передача сообщений)
    static void static_on_request()
    {
        MyMqttI2C* inst = MyMqttI2C::instance();
        size_t queue_size = inst->_mqtt_outgoing_queue.size();

        if (queue_size <= 0)
        {
            // Если очередь пуста, сбрасываем сигнал прерывания и отправляем dummy-сообщение
            inst->_set_interrupt_signal(false);
            inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessage::get_dummy(inst->_address)), MqttMessage::get_size_of());
        }
        else
        {
            // Отправка первого сообщения очереди
            MqttMessage outgoing_message = inst->_mqtt_outgoing_queue.front();
            inst->_mqtt_outgoing_queue.pop();

            size_t queue_size = inst->_mqtt_outgoing_queue.size();

            if (queue_size == 0)
                inst->_set_interrupt_signal(false);

            outgoing_message.set_has_a_following_messages(queue_size > 0);

            inst->_i2c->write(reinterpret_cast<uint8_t*>(&outgoing_message), MqttMessage::get_size_of());
        }
    }

    // Установка состояния сигнала прерывания (используется на стороне слейва)
    void _set_interrupt_signal(bool state)
    {
        if (!_is_master)
            digitalWrite(_interrupt_pin, state);
    }

    // Получение состояния сигнала прерывания для указанного адреса
    bool _get_interrupt_signal(uint8_t address = 0x01)
    {
        if (!_is_master)
        {
            return digitalRead(_interrupt_pin);
        }
        else if (address > 0x01 && _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            return digitalRead(_master_side_subscriptions[address].interrupt_pin);
        }
        else if (!_master_side_subscriptions.empty())
        {
            for (auto& sub_pair : _master_side_subscriptions)
                if (digitalRead(sub_pair.second.interrupt_pin))
                    return true;
            return false;
        }
        else
        {
            return false;
        }
    }

    // Проверка подписчиков на стороне мастера – получение входящих сообщений от слейвов
    void read_subscribers()
    {
        if (!_is_master)
            return;

        // Обновляем флаги прерывания
        static_interrupt_wrapper();

        // Для каждого подписчика, если флаг установлен, запрашиваем сообщение по I2C
        for (auto& sub_pair : _master_side_subscriptions)
        {
            // while (sub.second.is_master_called = digitalRead(sub.second.interrupt_pin))
            while (sub_pair.second.is_master_called)
            {
                _i2c->requestFrom(sub_pair.second.address, MqttMessage::get_size_of());
                delayMicroseconds(100);
                if (_i2c->available())
                {
                    MqttMessage incoming;
                    _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessage::get_size_of());
                    noInterrupts();
                    _mqtt_incoming_queue.push(incoming);
                    interrupts();

                    // Если в сообщении не установлен флаг, завершаем цикл для данного подписчика
                    if (!incoming.get_has_a_following_messages())
                    {
                        sub_pair.second.is_master_called = false;
                        break;
                    }
                }
                else
                {
                    sub_pair.second.is_master_called = false;
                    break;
                }
            }
        }
    }

    // Метод обработки входящих сообщений
    void update_incoming()
    {
        // Если устройство – мастер, маршрутизируем сообщения к соответствующим подписчикам
        if (_is_master)
        {
            noInterrupts();
            while (!_mqtt_incoming_queue.empty())
            {
                MqttMessage message = _mqtt_incoming_queue.front();
                _mqtt_incoming_queue.pop();
                interrupts();

                // По адресу сообщения ищем соответствующего подписчика
                auto it = _master_side_subscriptions.find(message.addr);
                if (it != _master_side_subscriptions.end())
                {
                    // Если найден, перебираем его обработчики и вызываем тот, что соответствует команде
                    for (auto &handler_pair : it->second.command_handlers)
                    {
                        if (handler_pair.first == message.get_command())
                        {
                            handler_pair.second(message);
                        }
                    }
                }
                else
                {
                    noInterrupts();
                }
            }
            interrupts();
        }
        else // Если устройство – слейв, перебираем все входящие сообщения и вызываем обработчики
        {
            noInterrupts();
            while (!_mqtt_incoming_queue.empty())
            {
                MqttMessage message = _mqtt_incoming_queue.front();
                _mqtt_incoming_queue.pop();
                interrupts();

                for (auto &handler_pair : _slave_side_command_handlers)
                {
                    if (handler_pair.first == message.get_command())
                    {
                        handler_pair.second(message);
                    }
                }
            }
        }
    }

    // Обработка исходящих сообщений
    void update_outgoing()
    {
        if (_is_master)
        {
            size_t queue_size = _mqtt_outgoing_queue.size();
            while (queue_size-- > 0)
            {
                MqttMessage outgoing = _mqtt_outgoing_queue.front();
                _mqtt_outgoing_queue.pop();

                _i2c->beginTransmission(outgoing.addr);
                _i2c->write(reinterpret_cast<uint8_t*>(&outgoing), MqttMessage::get_size_of());
                _i2c->endTransmission();

                delayMicroseconds(100);
            } 
        }
        else if (!_mqtt_outgoing_queue.empty())
        {
            _set_interrupt_signal(true);
        }
    }

public:
    // Получение экземпляра синглтона
    static MyMqttI2C* instance()
    {
        static MyMqttI2C inst;
        return &inst;
    }

    // Инициализация I2C-интерфейса
    TwoWire* begin(uint8_t sda, uint8_t scl, unsigned int freq, bool is_master, uint8_t addr = 0x01, char interrupt_pin = -1)
    {
        if (this->_is_master = is_master)
        {
#ifdef DEV_SSPS3_IS_MASTER
            _i2c = new TwoWire(0);
            _i2c->begin(sda, scl, freq);      
#endif
        }
        else
        {
#ifdef DEV_SSPS3_IS_SLAVE
            _interrupt_pin = interrupt_pin;
            // Если адрес меньше или равен 0x01, выбираем 0x02 по умолчанию (т.к. 0x01 у мастера)
            _address = (addr <= 0x01) ? 0x02 : addr;
            _i2c = new TwoWire(sda, scl);
            _i2c->begin(_address);
            _i2c->setClock(freq);
            
            if (_interrupt_pin != -1)
            {
                pinMode(_interrupt_pin, OUTPUT);
                digitalWrite(_interrupt_pin, LOW);
            }

            _i2c->onReceive(static_on_receive);
            _i2c->onRequest(static_on_request);
#endif
        }
        
        return _i2c;
    }

    // Регистрация обработчика для определённой команды
    bool set_after_receive_handler(uint8_t command, AfterReceiveEvent handler, uint8_t address = 0x01)
    {
        if (_is_master && _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            _master_side_subscriptions[address].command_handlers.push_back({ command, handler });
            return true;
        }
        else if (!this->_is_master)
        {
            _slave_side_command_handlers.push_back({ command, handler });
            return true;
        }
        return false;
    }

    // Подписка слейв-устройства на стороне мастера
    void subscribe_slave(uint8_t address, uint8_t master_to_slave_interrupt_pin)
    {
        if (!this->_is_master)
            return;

        _master_side_subscriptions[address] = MqttSlaveSubscriber(address, master_to_slave_interrupt_pin);

        pinMode(_master_side_subscriptions[address].interrupt_pin, INPUT_PULLDOWN);

        // Прикрепляем обработчик прерывания для данного пина
        attachInterrupt(digitalPinToInterrupt(_master_side_subscriptions[address].interrupt_pin),
                        MyMqttI2C::instance()->static_interrupt_wrapper,
                        CHANGE);
    }

    // Добавление сообщения в очередь исходящих сообщений
    bool push_message(MqttMessage new_message, uint8_t address = 0x01)
    {
        if (_is_master && address > 0x01 && _master_side_subscriptions.find(address) != _master_side_subscriptions.end())
        {
            new_message.set_addr(address);
            _mqtt_outgoing_queue.push(new_message);
            return true;
        }
        else if (!_is_master)
        {
            _mqtt_outgoing_queue.push(new_message);
            _set_interrupt_signal(true);
            return true;
        }
        return false;
    }

    // Основной метод обновления: обрабатывает подписчиков, входящие и исходящие сообщения
    void update()
    {
        read_subscribers();
        update_incoming();
        update_outgoing();
    }
};

#endif
