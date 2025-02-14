#pragma once
#ifndef NEW_MQTT_I2C_H
#define NEW_MQTT_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <functional>
#include <unordered_map>

// Определение констант для фиксированных размеров буферов и таймаута
constexpr size_t MAX_MESSAGE_QUEUE = 10;         // Максимальное количество сообщений в очереди
constexpr size_t MAX_SLAVE_SUBSCRIBERS = 5;        // Максимальное количество подписанных slave-ов
constexpr unsigned long I2C_TIMEOUT_MS = 150;       // Таймаут в миллисекундах для операций I2C

// Отключаем выравнивание для структуры сообщений
#pragma pack(push, 1)
struct MqttMessage {
    uint8_t command;      // Младший бит – X-бит (1: ещё есть, 0: последнее)
    uint8_t addr;
    uint8_t content[2];

    // Метод для установки/сброса X-бита
    void setXBit(bool more) {
        if(more)
            command |= 0x01;
        else
            command &= ~0x01;
    }
};
#pragma pack(pop)

// Перечисление команд
enum MqttCommand : uint8_t {
    GET_D_IO,
    GET_A_IN,
    GET_KB,
    SET_A_OUT,
    SET_R_OUT,
    CMD_NAN
};

// Структура для хранения информации о подписанном slave-е
struct SlaveSubscriber {
    uint8_t address;
    int notifyPin;
    volatile bool isAwaitReading;
};

class MQTT {
public:
    // Получение экземпляра синглтона
    static MQTT& getInstance() {
        static MQTT instance;
        return instance;
    }

    // Первая перегрузка begin()
    void begin(bool isMaster, int notifyPin = -1, uint32_t i2cFreq = 100000) {
        _isMaster = isMaster;
        initNotifyPin(notifyPin);
        initI2C(i2cFreq, -1, -1);
    }

    // Вторая перегрузка begin() с указателем на TwoWire
    void begin(TwoWire* wire, bool isMaster, int notifyPin = -1, uint32_t i2cFreq = 100000) {
        _isMaster = isMaster;
        initNotifyPin(notifyPin);
        initI2C(i2cFreq, -1, -1, wire);
    }

    // Третья перегрузка begin() с пинами SDA и SCL
    void begin(bool isMaster, int sda, int scl, uint32_t i2cFreq = 100000, int notifyPin = -1) {
        _isMaster = isMaster;
        initNotifyPin(notifyPin);
        initI2C(i2cFreq, sda, scl);
    }

    // Регистрация обработчика получения сообщений для заданной команды
    void registerOnReceived(MqttCommand cmd, std::function<void(const MqttMessage&)> handler) {
        _onReceivedHandlers[cmd] = handler;
    }
    // Регистрация обработчика запроса сообщений для заданной команды
    void registerOnRequest(MqttCommand cmd, std::function<void()> handler) {
        _onRequestHandlers[cmd] = handler;
    }

    // Подписка мастера на уведомления от slave-а с защитой критической секции
    void subscribeSlave(uint8_t addr, int notifyPin)
    {
        Serial.printf("Master: Подписка на slave 0x%X, пин %d\n", addr, notifyPin);

        bool canSubscribe = false;  // Флаг, разрешающий регистрацию прерывания
        noInterrupts();             // Отключаем прерывания для атомарного обновления массива
        if(_slaveCount < MAX_SLAVE_SUBSCRIBERS) {
            _slaveSubscribers[_slaveCount].address = addr;
            _slaveSubscribers[_slaveCount].notifyPin = notifyPin;
            _slaveSubscribers[_slaveCount].isAwaitReading = false;
            _slaveCount++;
            canSubscribe = true;
        }
        interrupts();               // Включаем прерывания
        if(canSubscribe) {
            // Вызываем attachInterrupt() уже с включёнными прерываниями
            _master_pin_intrpt_await = notifyPin;
            pinMode(notifyPin, INPUT_PULLUP);
            attachInterrupt(digitalPinToInterrupt(notifyPin), MQTT::slaveInterruptWrapper, RISING);
        }
    }

    // Метод для добавления сообщения в очередь (используется slave-ом)
    void pushMessage(const MqttMessage& msg) {
        noInterrupts();
        // Устанавливаем X-бит для всех уже накопленных сообщений
        for (size_t i = 0; i < _messageCount; i++) {
            _messageQueue[i].setXBit(true);
        }
        // Если в очереди ещё есть место, добавляем новое сообщение
        if (_messageCount < MAX_MESSAGE_QUEUE) {
            MqttMessage newMsg = msg;
            newMsg.setXBit(false); // Последнее сообщение в очереди
            _messageQueue[_messageCount] = newMsg;
            _messageCount++;

            if (!_isMaster && _slave_notify_pin != -1) {
                digitalWrite(_slave_notify_pin, HIGH);
            }
        }
        interrupts();
    }

    // Метод update() для мастера с ограничением по количеству пакетов и таймаутом
    void update(uint8_t maxPackets = 5) {
        if (!_isMaster) return;
        for (size_t i = 0; i < _slaveCount; i++) {
            if (_slaveSubscribers[i].isAwaitReading) {
                Serial.println("Сигнал от slave");
                uint8_t packetsRead = 0;
                unsigned long startTime = millis();
                while (packetsRead < maxPackets)
                {
                    uint8_t received = _wire->requestFrom(_slaveSubscribers[i].address, (uint8_t)sizeof(MqttMessage));
                    Serial.printf("Запрос данных от slave 0x%X\n", _slaveSubscribers[i].address);

                    if (received == 0) {
                        Serial.println("Ошибка: slave не ответил");
                        continue;
                    }

                    if (_wire->available() < (int)sizeof(MqttMessage))
                        break;

                    if (_wire->available() == sizeof(MqttMessage))
                    {
                        Serial.println("Валидное количество данных!");
                    }
                    else
                    {
                        Serial.println("Неверное количество данных...");
                    }

                    MqttMessage msg;
                    _wire->readBytes(reinterpret_cast<uint8_t*>(&msg), sizeof(MqttMessage));

                    auto it = _onReceivedHandlers.find((MqttCommand)(msg.command & 0xFE));
                    if (it != _onReceivedHandlers.end())
                        it->second(msg);
                    packetsRead++;
                    if (!(msg.command & 0x01)) {
                        _slaveSubscribers[i].isAwaitReading = false;
                        break;
                    }
                    if (millis() - startTime > I2C_TIMEOUT_MS) {
                        break;
                    }
                }
            }
        }
    }

    // Метод для отправки команды slave-у от мастера
    void sendCommand(uint8_t slaveAddress, MqttCommand cmd, const uint8_t* data = nullptr, size_t dataSize = 0) {
        _wire->beginTransmission(slaveAddress);
        uint8_t header = static_cast<uint8_t>(cmd);
        _wire->write(header);
        if (data && dataSize > 0)
            _wire->write(data, dataSize);
        _wire->endTransmission();
    }

    // Установка адреса slave-а
    void setSlaveAddress(uint8_t addr) {
        _slave_address = addr;
    }

private:
    // Приватный конструктор с настройкой обработчиков I2C
    MQTT() : _isMaster(true), _master_pin_intrpt_await(-1), _slave_notify_pin(-1),
             _slave_address(0), _wire(&Wire), _messageCount(0), _slaveCount(0)
    {}
    MQTT(const MQTT&) = delete;
    MQTT& operator=(const MQTT&) = delete;

    // Вспомогательный метод для настройки notify-пина
    void initNotifyPin(int notifyPin) {
        if (notifyPin != -1)
        {
            if (!_isMaster)
            {
                _slave_notify_pin = notifyPin;
                pinMode(notifyPin, OUTPUT);
            }
        }
    }
    // Унифицированная инициализация I2C, выбор между begin() с пинами или без
    void initI2C(uint32_t i2cFreq, int sda, int scl, TwoWire* wire = nullptr)
    {
        if (_isMaster)
        {
#ifdef DEV_SSPS3_IS_MASTER
            if (wire != nullptr)
            {
                this->_wire = wire;
            }
            else if (sda != -1 && scl != -1)
            {
                // Используем I2C шину 0 с указанными пинами
                this->_wire = new TwoWire(0);
                this->_wire->begin(sda, scl);
            }
            else
            {
                // Используем стандартную шину Wire
                this->_wire = &Wire;
                this->_wire->begin();
            }
            this->_wire->setClock(i2cFreq);
#else   
            // Логика для других мастер-устройств
#endif
        }
        else
        {
#ifdef DEV_SSPS3_IS_MASTER
            // здесь логика, если ESP32 выступает в качества SLAVE       
#else
            if (wire != nullptr)
                this->_wire = wire;
            else if (sda != -1 && scl != -1)
                this->_wire = new TwoWire(sda, scl);
            else
                this->_wire = &Wire;
#endif
            _wire->onReceive(MQTT::onReceiveStatic);
            _wire->onRequest(MQTT::onRequestStatic);
            _wire->begin(_slave_address);
        }

        _wire->setClock(i2cFreq);
    }
    // Статическая обёртка для вызова обработчика прерывания slave-а
    static void slaveInterruptWrapper() {
        MQTT::getInstance().handleSlaveInterrupt();
    }
    // Обработка прерываний: перебор подписанных slave-ов и установка флага, если на notify-пине высокий уровень
    void handleSlaveInterrupt() {
        for (size_t i = 0; i < _slaveCount; i++) {
            if (digitalRead(_slaveSubscribers[i].notifyPin) == HIGH)
                _slaveSubscribers[i].isAwaitReading = true;
        }
    }
    // Статическая обёртка для onReceive
    static void onReceiveStatic(int numBytes) {
        MQTT::getInstance().onReceive(numBytes);
    }
    // Приём данных по I2C и делегирование вызова обработчику
    void onReceive(int numBytes) {
        if (numBytes < (int)sizeof(MqttMessage))
            return;
        MqttMessage msg;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&msg);
        for (size_t i = 0; i < sizeof(MqttMessage); i++) {
            if(_wire->available())
                ptr[i] = _wire->read();
        }
        auto it = _onReceivedHandlers.find((MqttCommand)(msg.command & 0xFE));
        if(it != _onReceivedHandlers.end())
            it->second(msg);
    }
    // Статическая обёртка для onRequest
    static void onRequestStatic() {
        MQTT::getInstance().onRequest();
    }
    // Обработка запроса данных от мастера: отправка сообщения из очереди или dummy-пакета
    void onRequest() {
        if (_messageCount == 0) {
            MqttMessage dummy;
            dummy.command = CMD_NAN;
            dummy.addr = _slave_address;
            dummy.content[0] = 0;
            dummy.content[1] = 0;
            dummy.setXBit(false);
            _wire->write(reinterpret_cast<uint8_t*>(&dummy), sizeof(MqttMessage));
            if (_slave_notify_pin != -1)
                digitalWrite(_slave_notify_pin, LOW);
        } else {
            _wire->write(reinterpret_cast<uint8_t*>(&_messageQueue[0]), sizeof(MqttMessage));
            noInterrupts();
            for (size_t i = 1; i < _messageCount; i++) {
                _messageQueue[i - 1] = _messageQueue[i];
            }
            _messageCount--;
            interrupts();
            if (_messageCount == 0 && _slave_notify_pin != -1)
                digitalWrite(_slave_notify_pin, LOW);
        }
    }

    bool _isMaster;
    int _master_pin_intrpt_await;
    int _slave_notify_pin;
    uint8_t _slave_address;
    TwoWire* _wire;
    MqttMessage _messageQueue[MAX_MESSAGE_QUEUE];
    size_t _messageCount;
    SlaveSubscriber _slaveSubscribers[MAX_SLAVE_SUBSCRIBERS];
    size_t _slaveCount;
    std::unordered_map<MqttCommand, std::function<void(const MqttMessage&)>> _onReceivedHandlers;
    std::unordered_map<MqttCommand, std::function<void()>> _onRequestHandlers;
};

#endif