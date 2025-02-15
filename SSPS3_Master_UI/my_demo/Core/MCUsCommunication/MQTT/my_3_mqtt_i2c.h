#pragma once
#ifndef MY_MQTT_I2C_H
#define MY_MQTT_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include <unordered_map>
#include <cstring>     // Для memcpy, memset
#include <functional>

// Определение команд MQTT
enum MqttCommand : uint8_t {
    CMD_NAN,
    GET_D_IO,
    GET_A_IN,
    GET_KB,
    SET_A_OUT,
    SET_R_OUT
};

#pragma pack(push, 1)
// Структура сообщения MQTT
struct MqttMessage {
    uint8_t command = MqttCommand::CMD_NAN;  // Команда (с флагом наличия следующих сообщений в старшем бите)
    uint8_t addr    = 0x02;                  // Адрес получателя
    uint8_t content[2] = {0x00};             // Поле для данных

    MqttMessage() {}

    MqttMessage(uint8_t command, uint8_t addr) {
        this->command = command;
        this->addr = addr;
    }

    // Заполнение поля content данными, размер которых соответствует типу T
    template<typename T>
    void set_content(void* value_in) {
        memset(content, 0, sizeof(content));
        memcpy(content, value_in, sizeof(T));
    }

    // Извлечение данных из content в переменную типа T
    template<typename T>
    void get_content(T* value_out) {
        memcpy(reinterpret_cast<uint8_t*>(value_out), content, sizeof(T));
    }

    // Установка флага наличия следующих сообщений
    MqttMessage* set_has_a_following_messages(bool valid) {
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
    MqttMessage* set_addr(uint8_t addr) {
        this->addr = addr;
        return this;
    }

    // Возвращает указатель на dummy-сообщение с заданным адресом
    static MqttMessage* get_dummy(uint8_t addr) {
        static MqttMessage dummy;
        return dummy.set_addr(addr);
    }

    // Размер структуры сообщения
    static uint8_t get_size_of() {
        static uint8_t _size_of = sizeof(MqttMessage);
        return _size_of;
    }
};
#pragma pack(pop)

// Определения типов обработчиков событий
typedef std::function<void(MqttMessage)> AfterReceiveEvent;

// Структура подписчика слейва (на стороне мастера)
struct MqttSlaveSubscriber {
    uint8_t address = 0x02;         // Адрес слейва
    char interrupt_pin = -1;        // Номер пина для сигнала

    // Вектор пар: (команда, обработчик)
    std::vector<std::pair<uint8_t, AfterReceiveEvent>> command_handlers;
    
    MqttSlaveSubscriber() {}
    
    MqttSlaveSubscriber(uint8_t address, uint8_t interrupt_pin)
        : address(address), interrupt_pin(interrupt_pin) {}
};

// Реализация lock-free кольцевого буфера
template<typename T, uint8_t Size>
struct RingBuffer {
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;
    T buffer[Size];

    // Добавление элемента
    bool push(const T &item) {
        uint8_t nextTail = (tail + 1) % Size;
        if (nextTail == head) {
            // Буфер переполнен
            return false;
        }
        buffer[tail] = item;
        tail = nextTail;
        return true;
    }

    // Извлечение элемента (удаляет элемент из буфера)
    bool pop(T &item) {
        if (head == tail) {
            // Буфер пуст
            return false;
        }
        item = buffer[head];
        head = (head + 1) % Size;
        return true;
    }

    // Функция peek() возвращает элемент из начала буфера без удаления
    bool peek(T &item) const {
        if (head == tail) {
            return false;
        }
        item = buffer[head];
        return true;
    }

    // Проверка, пуст ли буфер
    bool empty() const {
        return head == tail;
    }

    // Количество элементов в буфере
    uint8_t count() const {
        if (tail >= head)
            return tail - head;
        else
            return Size - head + tail;
    }
};

#define INCOMING_BUFFER_SIZE 16
#define OUTGOING_BUFFER_SIZE 16

class MyMqttI2C {
private:
    // Приватный конструктор (синглтон)
    MyMqttI2C() {}

    // Подписки на стороне мастера: ключ – адрес слейва
    std::unordered_map<uint8_t, MqttSlaveSubscriber> _master_side_subscriptions;
    // Обработчики на стороне слейва
    std::vector<std::pair<uint8_t, AfterReceiveEvent>> _slave_side_command_handlers;
    // Кольцевые буферы для входящих и исходящих сообщений
    RingBuffer<MqttMessage, INCOMING_BUFFER_SIZE> _mqtt_incoming_buffer;
    RingBuffer<MqttMessage, OUTGOING_BUFFER_SIZE> _mqtt_outgoing_buffer;

    uint8_t _max_messages_per_subscription = 10;

    TwoWire* _i2c = nullptr;      // Интерфейс I2C
    short _sda = -1;
    short _scl = -1;
    unsigned int _freq = 400000;
    uint8_t _address = 0x01;      // Адрес устройства
    bool _is_master = false;      // Режим работы (мастер/слейв)
    char _interrupt_pin = -1;     // Пин для сигнала (на стороне слейва)

    volatile bool _slaveReceivedFlag = false;  // Флаг, устанавливаемый в onReceive при получении данных
    unsigned long _lastSlaveActivity = 0;        // Время последней активности

    void _recover_bus(void)
    {
        pinMode(_sda, INPUT);

        if (digitalRead(_sda) == LOW)
        {
            pinMode(_scl, OUTPUT);

            for (int i = 0; i < 20; i++)
            {
                digitalWrite(_scl, LOW);
                delayMicroseconds(10);
                digitalWrite(_scl, HIGH);
                delayMicroseconds(10);
            }

            pinMode(_scl, INPUT);
        }
#ifdef DEV_SSPS3_IS_MASTER
        _i2c->flush();
        _i2c->begin(_sda, _scl, _freq);
#endif
    }

    // Колбэк onReceive для I2C (вызывается в прерывании)
    static void static_on_receive(int numBytes) {
        MyMqttI2C* inst = MyMqttI2C::instance();
        uint8_t msgSize = MqttMessage::get_size_of();

        if (inst->_i2c->available() >= msgSize &&
            inst->_i2c->available() % msgSize == 0)
        {
            inst->_slaveReceivedFlag = true;

            while (numBytes >= msgSize) {
                MqttMessage incoming_message;
                inst->_i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming_message), msgSize);
                noInterrupts();
                inst->_mqtt_incoming_buffer.push(incoming_message);
                interrupts();
                numBytes -= msgSize;
            }
        } else {
            inst->_i2c->flush();
        }
    }

    // Колбэк onRequest для I2C (передача сообщений)
    // Изменения:
    // 1. Используем peek() для получения сообщения без удаления.
    // 2. Если отправка успешна, сообщение удаляется из буфера.
    // 3. Если отправка не удалась (error -1), сообщение остается для повторной отправки.
    static void static_on_request() {
        MyMqttI2C* inst = MyMqttI2C::instance();
        MqttMessage outgoing_message;
        noInterrupts();
        bool hasMessage = inst->_mqtt_outgoing_buffer.peek(outgoing_message);
        uint8_t remaining = inst->_mqtt_outgoing_buffer.count();
        interrupts();

        if (!hasMessage) {
            inst->_set_interrupt_signal(false);
            inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessage::get_dummy(inst->_address)), MqttMessage::get_size_of());
        } else {
            if (remaining == 0)
                inst->_set_interrupt_signal(false);
            outgoing_message.set_has_a_following_messages(remaining > 1);
            size_t bytesWritten = inst->_i2c->write(reinterpret_cast<uint8_t*>(&outgoing_message), MqttMessage::get_size_of());
            // Если отправка прошла успешно, удаляем сообщение из буфера
            if (bytesWritten == MqttMessage::get_size_of()) {
                noInterrupts();
                MqttMessage dummy;
                inst->_mqtt_outgoing_buffer.pop(dummy);
                interrupts();
            }
            // Иначе (например, error == -1) сообщение остаётся для повторной отправки,
            // а для восстановления шины вызываем встроенный метод recoverBus()
            else {
                inst->_recover_bus();
            }
        }
    }

    // Установка состояния сигнала (на стороне слейва)
    void _set_interrupt_signal(bool state) {
        if (!_is_master)
            digitalWrite(_interrupt_pin, state);
    }

    // Получение состояния сигнала по пину
    bool _get_interrupt_signal(uint8_t address = 0x01) {
        if (!_is_master) {
            return digitalRead(_interrupt_pin);
        } else if (address > 0x01 && _master_side_subscriptions.find(address) != _master_side_subscriptions.end()) {
            return digitalRead(_master_side_subscriptions[address].interrupt_pin);
        } else if (!_master_side_subscriptions.empty()) {
            for (auto& sub_pair : _master_side_subscriptions)
                if (digitalRead(sub_pair.second.interrupt_pin))
                    return true;
            return false;
        } else {
            return false;
        }
    }

    // Проверка подписчиков на стороне мастера – получение входящих сообщений от слейва
    //void read_subscribers() {
    //    if (!_is_master)
    //        return;
    //    for (auto& sub_pair : _master_side_subscriptions) {
    //        uint8_t messagesReceived = 0;
    //        while (digitalRead(sub_pair.second.interrupt_pin) && messagesReceived < _max_messages_per_subscription) {
    //            delayMicroseconds(80);
    //            _i2c->requestFrom(sub_pair.second.address, MqttMessage::get_size_of());
    //            delayMicroseconds(20);
    //            if (_i2c->available()) {
    //                MqttMessage incoming;
    //                _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessage::get_size_of());
    //                noInterrupts();
    //                _mqtt_incoming_buffer.push(incoming);
    //                interrupts();
    //                messagesReceived++;
    //                if (!incoming.get_has_a_following_messages())
    //                    break;
    //            } else {
    //                break;
    //            }
    //        }
    //    }
    //}
    // Проверка подписчиков на стороне мастера – получение входящих сообщений от слейва с обработкой ошибок через recoverBus()
    void read_subscribers()
    {
        if (!_is_master)
            return;
        for (auto& sub_pair : _master_side_subscriptions) {
            uint8_t messagesReceived = 0;
            // Пока пин сигнала активен и не достигнут лимит сообщений для одного прохода
            while (digitalRead(sub_pair.second.interrupt_pin) && messagesReceived < _max_messages_per_subscription) {
                delayMicroseconds(80);
                // Запрос данных от slave: ожидается размер одного сообщения
                int bytesRead = _i2c->requestFrom(sub_pair.second.address, MqttMessage::get_size_of());
                delayMicroseconds(20);
                // Если получено ожидаемое число байт, читаем сообщение
                if (bytesRead == MqttMessage::get_size_of()) {
                    MqttMessage incoming;
                    _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessage::get_size_of());
                    noInterrupts();
                    _mqtt_incoming_buffer.push(incoming);
                    interrupts();
                    messagesReceived++;
                    // Если в сообщении не установлен флаг наличия следующих, завершаем цикл для данного подписчика
                    if (!incoming.get_has_a_following_messages())
                        break;
                } else {
                    // Если получено некорректное число байт или произошла ошибка,
                    // вызываем встроенный метод recoverBus() для восстановления I²C-шины
                    this->_recover_bus();
                    break; // Выходим из цикла для этого подписчика
                }
            }
        }
    }


    // Обработка входящих сообщений
    void update_incoming() {
        if (_is_master) {
            MqttMessage message;
            noInterrupts();
            while (_mqtt_incoming_buffer.pop(message)) {
                interrupts();
                auto it = _master_side_subscriptions.find(message.addr);
                if (it != _master_side_subscriptions.end()) {
                    for (auto &handler_pair : it->second.command_handlers) {
                        if (handler_pair.first == message.get_command()) {
                            handler_pair.second(message);
                        }
                    }
                }
                noInterrupts();
            }
            interrupts();
        } else {
            MqttMessage message;
            noInterrupts();
            while (_mqtt_incoming_buffer.pop(message)) {
                interrupts();
                for (auto &handler_pair : _slave_side_command_handlers) {
                    if (handler_pair.first == message.get_command()) {
                        handler_pair.second(message);
                    }
                }
                noInterrupts();
            }
            interrupts();
        }
    }

    // Обработка исходящих сообщений для мастера
    // Здесь отправка сообщения выполняется и проверяется код ошибки, чтобы при error -1 вызвать recoverBus()
    void update_outgoing() {
        if (_is_master) {
            MqttMessage outgoing;
            noInterrupts();
            while (_mqtt_outgoing_buffer.pop(outgoing)) {
                interrupts();
                _i2c->beginTransmission(outgoing.addr);
                _i2c->write(reinterpret_cast<uint8_t*>(&outgoing), MqttMessage::get_size_of());
                int error = _i2c->endTransmission();
                // Если возникла ошибка -1, восстанавливаем шину и возвращаем сообщение в буфер
                if (error == -1) {
                    this->_recover_bus();
                    noInterrupts();
                    _mqtt_outgoing_buffer.push(outgoing);
                    interrupts();
                }
                delayMicroseconds(80);
                noInterrupts();
            }
            interrupts();
        } else if (!_mqtt_outgoing_buffer.empty()) {
            _set_interrupt_signal(true);
        }
    }

public:
    // Получение экземпляра синглтона
    static MyMqttI2C* instance() {
        static MyMqttI2C inst;
        return &inst;
    }

    // Инициализация I2C-интерфейса
    // Обратите внимание: метод recover() удалён, поскольку используется встроенный TwoWire::recoverBus()
    TwoWire* begin(uint8_t sda, uint8_t scl, unsigned int freq, bool is_master, uint8_t addr = 0x01, char interrupt_pin = -1) {
        _is_master = is_master;
        if (_is_master) {
#ifdef DEV_SSPS3_IS_MASTER
            _i2c = new TwoWire(0);
            _i2c->begin(
                _sda = sda,
                _scl = scl,
                _freq = freq
            );
#endif
        } else {
#ifndef DEV_SSPS3_IS_MASTER
            _interrupt_pin = interrupt_pin;
            _address = (addr <= 0x01) ? 0x02 : addr;
            _i2c = new TwoWire(
                _sda = sda,
                _scl = scl
            );
            _i2c->begin(_address);
            _i2c->setClock(_freq = freq);
            if (_interrupt_pin != -1) {
                pinMode(_interrupt_pin, OUTPUT);
                digitalWrite(_interrupt_pin, LOW);
            }
            _i2c->onReceive(static_on_receive);
            _i2c->onRequest(static_on_request);
#endif
        }
        return _i2c;
    }

    // Установка максимального числа сообщений для обработки одним проходом read_subscribers() на мастере
    void set_max_messages_per_subscription(uint8_t maxMessages) {
        _max_messages_per_subscription = maxMessages;
    }

    // Регистрация обработчика для определённой команды
    bool set_after_receive_handler(uint8_t command, AfterReceiveEvent handler, uint8_t address = 0x01) {
        if (_is_master && _master_side_subscriptions.find(address) != _master_side_subscriptions.end()) {
            _master_side_subscriptions[address].command_handlers.push_back({ command, handler });
            return true;
        } else if (!_is_master) {
            _slave_side_command_handlers.push_back({ command, handler });
            return true;
        }
        return false;
    }

    // Подписка слейв-устройства на стороне мастера
    void subscribe_slave(uint8_t address, uint8_t master_to_slave_interrupt_pin) {
        if (!_is_master)
            return;
        pinMode(master_to_slave_interrupt_pin, INPUT_PULLDOWN);
        _master_side_subscriptions[address] = MqttSlaveSubscriber(address, master_to_slave_interrupt_pin);
    }

    // Добавление сообщения в буфер исходящих сообщений
    bool push_message(MqttMessage new_message, uint8_t address = 0x01) {
        if (_is_master && address > 0x01 && _master_side_subscriptions.find(address) != _master_side_subscriptions.end()) {
            new_message.set_addr(address);
            noInterrupts();
            bool result = _mqtt_outgoing_buffer.push(new_message);
            interrupts();
            return result;
        } else if (!_is_master) {
            new_message.set_addr(this->_address);
            noInterrupts();
            bool result = _mqtt_outgoing_buffer.push(new_message);
            interrupts();
            _set_interrupt_signal(true);
            return result;
        }
        return false;
    }

    // Основной метод обновления: обработка подписчиков, входящих и исходящих сообщений
    //void update() {
    //    read_subscribers();
    //    update_incoming();
    //    update_outgoing();
    //}

    // Основной метод обновления: обработка подписчиков, входящих и исходящих сообщений,
    // а также восстановление I2C-шины на стороне slave по флагу активности.
    void update()
    {
        read_subscribers();
        update_incoming();
        update_outgoing();

        // Логика восстановления шины для slave
        if (!_is_master) {
            static const unsigned long inactivityThreshold = 5000; // Порог неактивности: 5000 мс
            unsigned long currentMillis = millis();

            // Если в onReceive была получена активность, флаг _slaveReceivedFlag устанавливается в true,
            // здесь обновляем время последней активности и сбрасываем флаг.
            if (_slaveReceivedFlag) {
                _lastSlaveActivity = currentMillis;
                _slaveReceivedFlag = false;
            }

            // Если с момента последней активности прошло больше порогового времени, выполняем восстановление шины:
            if (currentMillis - _lastSlaveActivity > inactivityThreshold) {
                Serial.println("recovery");
                // Восстанавливаем I2C-шину (функция _recover_bus() реализует генерацию тактов SCL и т.п.)
                //_recover_bus();
                // Переинициализируем I2C для slave
                 
                _i2c->flush();
                _i2c->begin(_address);
                _i2c->setClock(_freq);
                // Обновляем время последней активности после восстановления
                _lastSlaveActivity = currentMillis;
            }
        }
    }
};

#endif
