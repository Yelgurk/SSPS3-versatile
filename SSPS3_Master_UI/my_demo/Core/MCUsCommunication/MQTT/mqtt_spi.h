#pragma once
#ifndef MQTT_SPI_H
#define MQTT_SPI_H

#ifdef DEV_SSPS3_RUN_ON_PLC
#include "../Interface/i_mqtt_spi.h"
#else
#include "./i_mqtt_spi.h"
#endif

class MqttSPI : public IMqttSPI
{
private:
    SPI_CH*     _spi;
    SPI_CONF    _spi_conf;
    bool        _is_master;
    int         _address;
    int         _slave_intrpt_pin;
    int         _master_intrpt_await_pin;
    int         _cs_pin;

    uint8_t     _listener_rx_buffer[sizeof(MqttMessageSPI)];
    bool        _packet_received_flag;

    std::vector<MqttOutgoingMessageSPI> _send_queue;
    MqttMessageSPI                      _pending_message;
    uint8_t                             _tx_seq_counter;

    unsigned long   _global_ack_timeout;
    uint8_t         _global_max_retries;

    std::vector<std::pair<uint8_t, MqttReceivedSPICommandHandler>> _handlers;

    MqttSPI() :
        _spi(nullptr),
        _spi_conf(DEFAULT_SPI_FRQ, MSBFIRST, SPI_MODE0),
        _is_master(true),
        _address(0),
        _slave_intrpt_pin(-1),
        _master_intrpt_await_pin(-1),
        _cs_pin(10),
        _packet_received_flag(false),
        _tx_seq_counter(0),
        _global_ack_timeout(100),
        _global_max_retries(3)
    {}

    MqttSPI(const MqttSPI&) = delete;
    MqttSPI& operator=(const MqttSPI&) = delete;

    // Низкоуровневая отправка сообщения по _spi целым пакетом.
    // Для мастера инициируется _spi-транзакция; для slave сообщение сохраняется в _pending_message.
    void sendMessageInternal(const MqttMessageSPI& msg)
    {
        if (_is_master)
        {
            digitalWrite(_cs_pin, LOW);
            _spi->beginTransaction(_spi_conf);

            const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);

            for (size_t i = 0; i < sizeof(MqttMessageSPI); i++)
                _spi->transfer(p[i]);

            _spi->endTransaction();
            digitalWrite(_cs_pin, HIGH);
        }
        else
        {
            _pending_message = msg;
        }
    }

    // Обработка полностью полученного пакета (целого MqttMessageSPI).
    void processReceivedPacket(const MqttMessageSPI& msg)
    {
        if (msg.start != START_DELIMITER || msg.end != END_DELIMITER)
            return;

        uint16_t crcCalc = calculateCRC(
            reinterpret_cast<const uint8_t*>(&msg),
            sizeof(msg) - sizeof(msg.crc) - sizeof(msg.end)
        );

        if (crcCalc != msg.crc)
        {
            sendAckNack(msg, false);
            return;
        }
        if (msg.cmd == CMD_ACK)
        {
            if (!_send_queue.empty() && msg.seq == _send_queue.front().msg.seq)
                _send_queue.erase(_send_queue.begin());
        }
        else if (msg.cmd == CMD_NACK)
        {
            if (!_send_queue.empty() && msg.seq == _send_queue.front().msg.seq)
                _send_queue.front().last_attempt = 0;
        }
        else
        {
            sendAckNack(msg, true);

            for (auto& handlerPair : _handlers)
            {
                if (handlerPair.first == msg.cmd)
                    handlerPair.second(msg);
            }
        }
    }

    // Отправка ACK или NACK в ответ на полученное сообщение.
    void sendAckNack(const MqttMessageSPI& receivedMsg, bool ack)
    {
        MqttMessageSPI ackMsg;

        ackMsg.start = START_DELIMITER;
        ackMsg.src = _address;
        ackMsg.dst = receivedMsg.src;
        ackMsg.cmd = ack ? CMD_ACK : CMD_NACK;
        ackMsg.seq = receivedMsg.seq;
        ackMsg.len = 0;
        ackMsg.crc = calculateCRC(
            reinterpret_cast<uint8_t*>(&ackMsg),
            sizeof(ackMsg) - sizeof(ackMsg.crc) - sizeof(ackMsg.end)
        );
        ackMsg.end = END_DELIMITER;

        sendMessageInternal(ackMsg);
    }

    // Асинхронный state-machine отправки.
    void updateSend()
    {
        if (_send_queue.empty()) return;

        MqttOutgoingMessageSPI& current = _send_queue.front();
        unsigned long now = millis();

        if (!current.waiting_ack)
        {
            current.last_attempt = now;
            current.waiting_ack = true;

            sendMessageInternal(current.msg);

            if (!_is_master && _slave_intrpt_pin != -1)
                digitalWrite(_slave_intrpt_pin, HIGH);
        }
        else
        {
            if (now - current.last_attempt >= current.ack_timeout)
            {
                if (current.retries < current.max_retries)
                {
                    current.retries++;
                    current.last_attempt = now;

                    sendMessageInternal(current.msg);

                    if (!_is_master && _slave_intrpt_pin != -1)
                        digitalWrite(_slave_intrpt_pin, HIGH);
                }
                else
                {
                    // Если число попыток исчерпано, сбрасываем waiting_ack, оставляя сообщение в очереди.
                    current.waiting_ack = false;

                    // Можно переместить сообщение в конец очереди для приоритета новых.
                    if (OUTGOING_MESSAGE_FAILED_PUSH_BACK)
                    {
                        MqttOutgoingMessageSPI failed = current;
                        _send_queue.erase(_send_queue.begin());
                        _send_queue.push_back(failed);
                    }
                }
            }
        }
    }

    // Асинхронный state-machine приёма целого пакета.
    void updateReceive()
    {
        if (_is_master)
        {
            if (_master_intrpt_await_pin != -1 && digitalRead(_master_intrpt_await_pin) == HIGH)
            {
                MqttMessageSPI msg;
                digitalWrite(_cs_pin, LOW);
                _spi->beginTransaction(_spi_conf);

                uint8_t* p = reinterpret_cast<uint8_t*>(&msg);

                for (size_t i = 0; i < sizeof(MqttMessageSPI); i++)
                    p[i] = _spi->transfer(0x00);

                _spi->endTransaction();
                digitalWrite(_cs_pin, HIGH);
                processReceivedPacket(msg);
            }
        }
        else
        {
            if (_packet_received_flag)
            {
                _packet_received_flag = false;
                MqttMessageSPI msg;
                memcpy(&msg, _listener_rx_buffer, sizeof(MqttMessageSPI));
                processReceivedPacket(msg);
            }
        }
    }

public:
    // Получение единственного экземпляра (singleton)
    static MqttSPI& getInstance()
    {
        static MqttSPI instance;
        return instance;
    }

    //-----------------------------------------------------------------
    // Инициализация _spi->
    // isMaster: true, если устройство является мастером, false – slave.
    // csPin: пин для Chip Select.
    // notifyPin: (опционально, для slave) пин, которым устройство уведомляет master о наличии данных.
    //-----------------------------------------------------------------
    SPI_CH* begin(bool isMaster, int csPin, int speed_hz = DEFAULT_SPI_FRQ, int notifyPin = -1) override
    {
        return begin(&SPI, isMaster, csPin, speed_hz, notifyPin);
    }

    SPI_CH* begin(int mosi, int miso, int sck, bool isMaster, int csPin, uint8_t spi_bus = HSPI, int speed_hz =  DEFAULT_SPI_FRQ, int notifyPin = -1) override
    {
        this->_spi = new SPI_CH(spi_bus);
        this->_spi->begin(sck, miso, mosi, csPin);

        return begin(this->_spi, isMaster, csPin, speed_hz, notifyPin);
    }

    SPI_CH* begin(SPI_CH* _spi, bool isMaster, int csPin, int speed_hz = DEFAULT_SPI_FRQ, int notifyPin = -1)
    {
        this->_spi = _spi;
        this->_is_master = isMaster;
        this->_cs_pin = csPin;
        pinMode(_cs_pin, OUTPUT);
        digitalWrite(_cs_pin, HIGH); // CS неактивен

        if (_is_master)
        {
            _spi_conf = SPI_CONF(speed_hz, MSBFIRST, SPI_MODE0);
            _spi->begin();
        }
        else
        {
            _spi->begin();

            if (notifyPin != -1)
            {
                _slave_intrpt_pin = notifyPin;
                pinMode(_slave_intrpt_pin, OUTPUT);
                digitalWrite(_slave_intrpt_pin, LOW);
            }
            _packet_received_flag = false;
        }
        _tx_seq_counter = 0;

        return this->_spi;
    }

    // Задание адреса данного устройства (0–255)
    void setAddress(uint8_t addr) override {
        _address = addr;
    }

    // Регистрация обработчика для заданного кода команды.
    void registerHandler(uint8_t command, MqttReceivedSPICommandHandler handler) {
        _handlers.push_back({ command, handler });
    }

    // Таймаут ожидания ACK (в мс)
    void setGlobalAckTimeout(unsigned long timeout) override
    {
        _global_ack_timeout = timeout;
    }

    // Максимальное количество повторных попыток отправки
    void setGlobalMaxRetries(uint8_t retries) override
    {
        _global_max_retries = retries;
    }

    // Асинхронное помещение сообщения в очередь на отправку.
    void queueMessage(
        uint8_t dst,
        uint8_t cmd,
        const uint8_t* data,
        uint8_t length,
        uint8_t max_retries = 0,
        unsigned long ack_timeout = 0) override
    {
        if (length > MAX_PAYLOAD_SIZE_SPI)
            return;

        MqttMessageSPI msg;
        msg.start = START_DELIMITER;
        msg.src = _address;
        msg.dst = dst;
        msg.cmd = cmd;
        msg.seq = _tx_seq_counter++;
        msg.len = length;
        memcpy(msg.payload, data, length);
        msg.crc = calculateCRC(
            reinterpret_cast<uint8_t*>(&msg),
            sizeof(msg) - sizeof(msg.crc) - sizeof(msg.end)
        );
        msg.end = END_DELIMITER;

        MqttOutgoingMessageSPI out;
        out.msg = msg;
        out.retries = 0;
        out.last_attempt = 0;
        out.max_retries = (max_retries == 0) ? _global_max_retries : max_retries;
        out.ack_timeout = (ack_timeout == 0) ? _global_ack_timeout : ack_timeout;
        out.waiting_ack = false;
        _send_queue.push_back(out);
    }

    // Основной асинхронный метод update(), который нужно вызывать в loop().
    void update() override
    {
        updateSend();
        updateReceive();
    }

    // Для мастера: задать пин, на который slave подаёт уведомление (notify-пин).
    void setSlaveNotifyPin(int pin) override
    {
        _master_intrpt_await_pin = pin;
        pinMode(_master_intrpt_await_pin, INPUT);
    }

    // Метод, вызываемый из _spi-прерывания (со стороны slave),
    // когда полностью получен пакет от master.
    void notifyPacketReceived(const uint8_t* data, size_t length) override
    {
        if (length != sizeof(MqttMessageSPI)) return;
        memcpy(_listener_rx_buffer, data, length);
        _packet_received_flag = true;
    }

    // Вычисление CRC-16 (алгоритм CRC-CCITT) для защиты данных.
    uint16_t calculateCRC(const uint8_t* data, size_t length) override
    {
        uint16_t crc = 0xFFFF;

        for (size_t i = 0; i < length; i++)
        {
            crc ^= static_cast<uint16_t>(data[i]) << 8;

            for (uint8_t j = 0; j < 8; j++)
            {
                if (crc & 0x8000)
                    crc = (crc << 1) ^ 0x1021;
                else
                    crc <<= 1;
            }
        }
        return crc;
    }
};

#endif // !MQTT_SPI_H
