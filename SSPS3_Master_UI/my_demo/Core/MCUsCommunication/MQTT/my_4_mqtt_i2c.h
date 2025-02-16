#pragma once
#ifndef MY_MQTT_I2C_H
#define MY_MQTT_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include <unordered_map>
#include <cstring>     // Для memcpy, memset
#include <functional>

// ================================
// 1. Перечисление команд MQTT – добавлены команды ACK и NACK для подтверждения
// ================================
enum MqttCommand : uint8_t {
    CMD_NAN,
    GET_D_IO,
    GET_A_IN,
    GET_KB,
    SET_A_OUT,
    SET_R_OUT,
    ACK,    // *** Изменение ACK/NACK: ACK – подтверждение успешного приёма
    NACK    // *** Изменение ACK/NACK: NACK – отрицательное подтверждение
};

#pragma pack(push, 1)
// ================================
// 2. Структура сообщения MQTT – добавлено поле seq для идентификации сообщений в ACK/NACK режиме
// ================================
struct MqttMessage {
    uint8_t command = MqttCommand::CMD_NAN;  // Команда (с флагом наличия следующих сообщений в старшем бите)
    uint8_t addr    = 0x02;                  // Адрес получателя
    uint8_t content[2] = {0x00};             // Поле для данных
    uint8_t seq = 0;                       // *** Изменение ACK/NACK: последовательный номер сообщения

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

    // Размер структуры сообщения – учитывает новое поле seq
    static uint8_t get_size_of() {
        static uint8_t _size_of = sizeof(MqttMessage);
        return _size_of;
    }
};
#pragma pack(pop)

// Определения типов обработчиков событий
typedef std::function<void(MqttMessage)> AfterReceiveEvent;

// ================================
// 3. Структура подписчика слейва (на стороне мастера)
// Добавлено новое поле restart_pin для хранения пина, с помощью которого мастер может инициировать рестарт I²C на слейве
// ================================
struct MqttSlaveSubscriber {
    uint8_t address = 0x02;         // Адрес слейва
    char interrupt_pin = -1;        // Пин для сигнала (уведомления о сообщениях)
    char restart_pin = -1;          // *** Изменение ACK/NACK и рестарта: пин для вызова рестарта I²C на стороне слейва

    // Вектор пар: (команда, обработчик)
    std::vector<std::pair<uint8_t, AfterReceiveEvent>> command_handlers;
    
    MqttSlaveSubscriber() {}
    
    // Конструктор с дополнительным параметром restart_pin
    MqttSlaveSubscriber(uint8_t address, uint8_t interrupt_pin, uint8_t restart_pin = -1)
        : address(address), interrupt_pin(interrupt_pin), restart_pin(restart_pin) {}
};

// Реализация lock-free кольцевого буфера
template<typename T, uint8_t Size>
struct RingBuffer {
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;
    T buffer[Size];

    // Добавление элемента. Функция не выполняет динамического выделения памяти.
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

    // Извлечение элемента
    bool pop(T &item) {
        if (head == tail) {
            // Буфер пуст
            return false;
        }
        item = buffer[head];
        head = (head + 1) % Size;
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
    uint8_t _address = 0x01;      // Адрес устройства
    bool _is_master = false;      // Режим работы (мастер/слейв)
    char _interrupt_pin = -1;     // Пин для сигнала (используется на стороне слейва)

    // Сохранённые параметры для возможного рестарта I2C (для мастера)
    uint8_t _sda = 0, _scl = 0;
    unsigned int _freq = 0;
    // Для слейва – пин, по которому вызывается рестарт I2C (если задан)
    int _i2cRestartCallPin = -1;

    // *** Изменение ACK/NACK: Флаг включения режима подтверждения
    bool _useAckNack = false;

    // *** Изменение ACK/NACK (мастерская часть): при отправке от master к slave сообщение остается в ожидании подтверждения
    // (при отключенном режиме ACK/NACK эти структуры не используются)
    std::unordered_map<uint8_t, MqttMessage> _master_pending_messages;          // Ключ – адрес slave
    std::unordered_map<uint8_t, unsigned long> _master_pending_timestamps;        // Время отправки для таймаута
    uint8_t _master_seq_counter = 0;  // Счётчик последовательных номеров для master

    // *** Изменение ACK/NACK (слейвовая часть): Для сообщений, отправляемых slave к master, необходимо сохранять pending-сообщение,
    // чтобы не удалять его, пока master не пришлёт ACK. Также pending-ответ slave для подтверждения получения от master.
    bool _slave_has_pending_outgoing = false;
    MqttMessage _slave_pending_outgoing;
    bool _slave_has_pending_response = false;
    MqttMessage _slave_pending_response;

    // Таймаут ожидания ACK (для master->slave)
    unsigned long _ackTimeout = 1000;   // в мс

    // ================================
    // Колбэк обработки события onReceive для I2C (вызывается в прерывании)
    // ================================
    static void static_on_receive(int numBytes) {
        MyMqttI2C* inst = MyMqttI2C::instance();
        uint8_t msgSize = MqttMessage::get_size_of();

        // Если устройство slave – обрабатываем входящие данные от master
        if (!inst->_is_master) {
            // Читаем все доступные байты
            while (inst->_i2c->available() >= msgSize) {
                MqttMessage incoming_message;
                inst->_i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming_message), msgSize);

                // *** Изменение ACK/NACK (слейв): Если режим включён, проверяем, является ли полученное сообщение ACK/NACK
                if (inst->_useAckNack) {
                    // Если команда ACK или NACK, то это ответ от master на ранее отправленное сообщение slave
                    if (incoming_message.get_command() == MqttCommand::ACK || incoming_message.get_command() == MqttCommand::NACK) {
                        // Сравниваем seq с pending-сообщением
                        if (inst->_slave_has_pending_outgoing && inst->_slave_pending_outgoing.seq == incoming_message.seq) {
                            if (incoming_message.get_command() == MqttCommand::ACK) {
                                // ACK получен – удаляем pending-сообщение
                                inst->_slave_has_pending_outgoing = false;
                            }
                            // Если получен NACK – оставляем pending, чтобы повторно отсылать
                        } 
                        // Если это ACK/NACK для ответа на сообщение, отправленное master,
                        // его обработка происходит на стороне master, здесь ничего не делаем.
                    } else {
                        // Если получено обычное сообщение от master, обрабатываем его
                        for (auto &handler_pair : inst->_slave_side_command_handlers) {
                            if (handler_pair.first == incoming_message.get_command()) {
                                handler_pair.second(incoming_message);
                            }
                        }
                        // Затем сразу формируем pending-ответ для master
                        MqttMessage response;
                        response.command = MqttCommand::ACK; // Если требуется можно добавить логику NACK при ошибке
                        response.addr = 0x01;   // Предполагаемый адрес мастера
                        response.seq = incoming_message.seq;
                        noInterrupts();
                        inst->_slave_pending_response = response;
                        inst->_slave_has_pending_response = true;
                        interrupts();
                    }
                } else {
                    // Режим без ACK/NACK – простая обработка
                    for (auto &handler_pair : inst->_slave_side_command_handlers) {
                        if (handler_pair.first == incoming_message.get_command()) {
                            handler_pair.second(incoming_message);
                        }
                    }
                }
            }
        } else {
            // Если устройство master, onReceive используется в другом контексте – обрабатываем как обычно
            if (inst->_i2c->available() >= msgSize && inst->_i2c->available() % msgSize == 0) {
                while (numBytes >= msgSize) {
                    MqttMessage incoming_message;
                    inst->_i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming_message), msgSize);
                    noInterrupts();
                    inst->_mqtt_incoming_buffer.push(incoming_message);
                    interrupts();
                    numBytes -= msgSize;
                }
            } else {
                while (inst->_i2c->available())
                    inst->_i2c->read();
            }
        }
    }

    // ================================
    // Колбэк обработки события onRequest для I2C (передача сообщений)
    // Вызывается на стороне slave, когда master запрашивает данные
    // ================================
    static void static_on_request() {
        MyMqttI2C* inst = MyMqttI2C::instance();
        uint8_t msgSize = MqttMessage::get_size_of();
        // На стороне slave, если используется ACK/NACK, сначала проверяем pending-ответ от master
        if (!inst->_is_master) {
            noInterrupts();
            if (inst->_useAckNack && inst->_slave_has_pending_response) {
                // *** Изменение ACK/NACK (слейв): Отправляем pending-ответ (ACK/NACK) от slave
                inst->_i2c->write(reinterpret_cast<uint8_t*>(&inst->_slave_pending_response), msgSize);
                inst->_slave_has_pending_response = false; // После отправки сбрасываем pending-ответ
                interrupts();
                return;
            }
            // Если нет pending-ответа, проверяем pending-сообщение для отправки slave->master
            if (inst->_useAckNack) {
                if (!inst->_slave_has_pending_outgoing) {
                    // Берём новое сообщение из очереди, но не удаляем его окончательно – ждем ACK от master
                    bool messageAvailable = inst->_mqtt_outgoing_buffer.pop(inst->_slave_pending_outgoing);
                    if (messageAvailable) {
                        inst->_slave_has_pending_outgoing = true;
                    }
                }
                if (inst->_slave_has_pending_outgoing) {
                    inst->_i2c->write(reinterpret_cast<uint8_t*>(&inst->_slave_pending_outgoing), msgSize);
                } else {
                    inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessage::get_dummy(inst->_address)), msgSize);
                }
            } else {
                // Режим без ACK/NACK – как было раньше: просто извлекаем сообщение из очереди
                MqttMessage outgoing_message;
                if (inst->_mqtt_outgoing_buffer.pop(outgoing_message)) {
                    inst->_i2c->write(reinterpret_cast<uint8_t*>(&outgoing_message), msgSize);
                } else {
                    inst->_i2c->write(reinterpret_cast<uint8_t*>(MqttMessage::get_dummy(inst->_address)), msgSize);
                }
            }
            interrupts();
        }
    }

    // ================================
    // Установка состояния сигнала (на стороне слейва)
    // ================================
    void _set_interrupt_signal(bool state) {
        if (!_is_master)
            digitalWrite(_interrupt_pin, state);
    }

    // ================================
    // Получение состояния сигнала по пину
    // ================================
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

    // ================================
    // Метод рестарта I2C для мастера
    // Вызывается при обнаружении ошибок (например, -1 или 263)
    // ================================
    void restart_i2c_bus() {
        if (_i2c) {
            _i2c->end();
            _i2c->flush();
            _i2c->begin(_sda, _scl, _freq);
            delay(100);
        }
    }

    // ================================
    // Метод рестарта I2C для слейва
    // Вызывается при срабатывании прерывания на пине i2cRestartCallPin
    // ================================
    void _restart_i2c_slave() {
        if (_i2c) {
            _i2c->end();
            _i2c->flush();
            _i2c->begin(_address);
            _i2c->setClock(_freq);
            _i2c->onReceive(static_on_receive);
            _i2c->onRequest(static_on_request);
            delay(100);
        }
    }

    // ================================
    // Статическая ISR-функция для рестарта I2C на слейве
    // ================================
    static void static_i2cRestartISR() {
        MyMqttI2C::instance()->_restart_i2c_slave();
    }

    // ================================
    // Обработка подписчиков на стороне мастера – получение входящих сообщений от slave
    // Если режим ACK/NACK включён, после успешного чтения master сразу отправляет ACK на адрес slave,
    // чтобы slave мог удалить сообщение из очереди и не отсылать его повторно.
    // ================================
    void read_subscribers() {
        if (!_is_master)
            return;
        for (auto& sub_pair : _master_side_subscriptions) {
            uint8_t messagesReceived = 0;
            while (digitalRead(sub_pair.second.interrupt_pin) && messagesReceived < _max_messages_per_subscription) {
                int bytesReceived = _i2c->requestFrom(sub_pair.second.address, MqttMessage::get_size_of());
                if (bytesReceived != MqttMessage::get_size_of()) {
                    restart_i2c_bus();
                    if(sub_pair.second.restart_pin != -1) {
                        digitalWrite(sub_pair.second.restart_pin, HIGH);
                        delay(10);
                        digitalWrite(sub_pair.second.restart_pin, LOW);
                    }
                    break;
                }
                if (_i2c->available() >= MqttMessage::get_size_of()) {
                    MqttMessage incoming;
                    _i2c->readBytes(reinterpret_cast<uint8_t*>(&incoming), MqttMessage::get_size_of());
                    
                    // *** Изменение ACK/NACK (master, slave->master):
                    // Если режим включён, сразу после получения отправляем ACK на slave, чтобы он удалил сообщение.
                    if(_useAckNack) {
                        _i2c->beginTransmission(incoming.addr);
                        MqttMessage ackMsg;
                        ackMsg.command = MqttCommand::ACK;
                        ackMsg.seq = incoming.seq;
                        _i2c->write(reinterpret_cast<uint8_t*>(&ackMsg), MqttMessage::get_size_of());
                        _i2c->endTransmission();
                    }
                    
                    noInterrupts();
                    _mqtt_incoming_buffer.push(incoming);
                    interrupts();
                    messagesReceived++;
                    if (!incoming.get_has_a_following_messages())
                        break;
                } else {
                    break;
                }
            }
        }
    }

    // ================================
    // Обработка входящих сообщений (у master и slave)
    // Здесь просто вызываются соответствующие обработчики для каждого полученного сообщения.
    // *** В master данная функция НЕ содержит логики ACK/NACK, т.к. подтверждения отправляются в read_subscribers()
    // ================================
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

    // ================================
    // Обработка исходящих сообщений
    // Для мастера, если включён режим ACK/NACK, после отправки сообщения выполняется запрос для получения ACK/NACK от slave.
    // Если ACK получен с правильным seq – сообщение считается доставленным, иначе – возвращается в очередь для повторной отправки.
    // Для slave логика остаётся прежней – просто выставляется сигнал.
    // ================================
    void update_outgoing() {
        if (_is_master) {
            if (_useAckNack) {
                // Обработка сообщений master->slave с механизмом подтверждения
                MqttMessage next;
                noInterrupts();
                bool hasMsg = _mqtt_outgoing_buffer.pop(next);
                interrupts();
                if (hasMsg) {
                    next.seq = _master_seq_counter++; // Присваиваем последовательный номер
                    _i2c->beginTransmission(next.addr);
                    _i2c->write(reinterpret_cast<uint8_t*>(&next), MqttMessage::get_size_of());
                    int result = _i2c->endTransmission();
                    if(result != 0) {
                        restart_i2c_bus();
                        if (_master_side_subscriptions[next.addr].restart_pin != -1) {
                            digitalWrite(_master_side_subscriptions[next.addr].restart_pin, HIGH);
                            delay(10);
                            digitalWrite(_master_side_subscriptions[next.addr].restart_pin, LOW);
                        }
                        // В случае ошибки возвращаем сообщение в очередь
                        noInterrupts();
                        _mqtt_outgoing_buffer.push(next);
                        interrupts();
                        return;
                    }
                    // Запрашиваем у slave подтверждение (ACK/NACK)
                    int ackBytes = _i2c->requestFrom(next.addr, MqttMessage::get_size_of());
                    bool ackOk = false;
                    if (ackBytes >= MqttMessage::get_size_of() && _i2c->available() >= MqttMessage::get_size_of()) {
                        MqttMessage ackMsg;
                        _i2c->readBytes(reinterpret_cast<uint8_t*>(&ackMsg), MqttMessage::get_size_of());
                        if (ackMsg.get_command() == MqttCommand::ACK && ackMsg.seq == next.seq) {
                            ackOk = true;
                        }
                    }
                    if (!ackOk) {
                        // Если не получен корректный ACK, возвращаем сообщение в очередь для повторной отправки
                        noInterrupts();
                        _mqtt_outgoing_buffer.push(next);
                        interrupts();
                    }
                }
            } else {
                // Режим без ACK/NACK – простая отправка
                MqttMessage outgoing;
                noInterrupts();
                while (_mqtt_outgoing_buffer.pop(outgoing)) {
                    interrupts();
                    _i2c->beginTransmission(outgoing.addr);
                    _i2c->write(reinterpret_cast<uint8_t*>(&outgoing), MqttMessage::get_size_of());
                    int result = _i2c->endTransmission();
                    if(result != 0) {
                        restart_i2c_bus();
                        if (_master_side_subscriptions[outgoing.addr].restart_pin != -1) {
                            digitalWrite(_master_side_subscriptions[outgoing.addr].restart_pin, HIGH);
                            delay(10);
                            digitalWrite(_master_side_subscriptions[outgoing.addr].restart_pin, LOW);
                        }
                    }
                    delayMicroseconds(80);
                    noInterrupts();
                }
                interrupts();
            }
        } else if (!_mqtt_outgoing_buffer.empty()) {
            _set_interrupt_signal(true);
        }
    }

public:
    // ================================
    // Получение экземпляра синглтона
    // ================================
    static MyMqttI2C* instance() {
        static MyMqttI2C inst;
        return &inst;
    }

    // ================================
    // Инициализация I2C-интерфейса
    // Изменён метод begin: теперь дополнительно принимается параметр i2cRestartCallPin (для слейва).
    // Сохраняются параметры sda, scl, freq для возможного рестарта.
    // Если устройство slave и i2cRestartCallPin != -1, выполняется attachInterrupt.
    // ================================
    TwoWire* begin(uint8_t sda, uint8_t scl, unsigned int freq, bool is_master, uint8_t addr = 0x01, char interrupt_pin = -1, int i2cRestartCallPin = -1) {
        _is_master = is_master;
        _sda = sda;         // *** Изменение рестарта: сохраняем sda
        _scl = scl;         // *** Изменение рестарта: сохраняем scl
        _freq = freq;       // *** Изменение рестарта: сохраняем частоту
        if (_is_master) {
#ifdef DEV_SSPS3_IS_MASTER
            _i2c = new TwoWire(0);
            _i2c->begin(sda, scl, freq);
#endif
        } else {
#ifndef DEV_SSPS3_IS_MASTER
            _interrupt_pin = interrupt_pin;
            _address = (addr <= 0x01) ? 0x02 : addr;
            _i2c = new TwoWire(sda, scl);
            _i2c->begin(_address);
            _i2c->setClock(freq);
            if (_interrupt_pin != -1) {
                pinMode(_interrupt_pin, OUTPUT);
                digitalWrite(_interrupt_pin, LOW);
            }
            // *** Изменение рестарта: если указан пин рестарта I2C, то вешаем прерывание
            _i2cRestartCallPin = i2cRestartCallPin;
            if (_i2cRestartCallPin != -1) {
                pinMode(_i2cRestartCallPin, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(_i2cRestartCallPin), static_i2cRestartISR, FALLING);
            }
            _i2c->onReceive(static_on_receive);
            _i2c->onRequest(static_on_request);
#endif
        }
        return _i2c;
    }

    // ================================
    // Установка максимального числа сообщений для обработки одним проходом read_subscribers() на мастере
    // ================================
    void set_max_messages_per_subscription(uint8_t maxMessages) {
        _max_messages_per_subscription = maxMessages;
    }

    // ================================
    // Регистрация обработчика для определённой команды
    // ================================
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

    // ================================
    // Подписка слейва на стороне мастера
    // Изменён метод subscribe_slave: дополнительно принимает параметр master_to_slave_restart_pin,
    // который сохраняется в экземпляр подписчика и используется для вызова рестарта I2C на слейве.
    // ================================
    void subscribe_slave(uint8_t address, uint8_t master_to_slave_notify_pin, uint8_t master_to_slave_restart_pin = -1) {
        if (!_is_master)
            return;
        pinMode(master_to_slave_notify_pin, INPUT_PULLDOWN);
        if(master_to_slave_restart_pin != -1){
            pinMode(master_to_slave_restart_pin, OUTPUT);
            digitalWrite(master_to_slave_restart_pin, LOW);
        }
        _master_side_subscriptions[address] = MqttSlaveSubscriber(address, master_to_slave_notify_pin, master_to_slave_restart_pin);
    }

    // ================================
    // Добавление сообщения в буфер исходящих сообщений
    // Для мастера при включённом режиме ACK/NACK сообщение будет ожидать подтверждения от slave.
    // ================================
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

    // ================================
    // Метод для включения/отключения режима ACK/NACK
    // ================================
    void useAckNack(bool enable) {
        _useAckNack = enable;
    }

    // ================================
    // Основной метод обновления: обработка подписчиков, входящих и исходящих сообщений
    // ================================
    void update() {
        read_subscribers();
        update_incoming();
        update_outgoing();
    }
};

#endif
