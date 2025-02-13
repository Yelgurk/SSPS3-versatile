#pragma once
#ifndef MQTT_I2C_H
#define MQTT_I2C_H

#ifdef DEV_SSPS3_RUN_ON_PLC
  #include "../Interface/i_mqtt_i2c.h"
#else
  #include "./i_mqtt_i2c.h"
#endif

class MqttI2C : public IMqttI2C {
private:
    I2C_CH* _i2c;
    bool    _is_master;
    int     _address;                     // Локальный адрес устройства (мастера или slave)
    int     _remote_address;              // Для мастера: адрес slave, с которого запрашивать данные
    int     _slave_intrpt_pin;            // Для slave: пин, которым slave уведомляет мастера (выставляется HIGH при наличии данных)
    int     _master_intrpt_await_pin;       // Для мастера: пин, по которому мастер ожидает уведомления от slave

    uint8_t _listener_rx_buffer[sizeof(MqttMessageI2C)];
    bool    _packet_received_flag;

    std::vector<MqttOutgoingMessageI2C> _send_queue;
    MqttMessageI2C                      _pending_message;
    uint8_t                               _tx_seq_counter;
    
    unsigned long   _global_ack_timeout;
    uint8_t         _global_max_retries;
    
    std::vector<std::pair<uint8_t, MqttReceivedI2CCommandHandler>> _handlers;
    
    MqttI2C() :
        _i2c(nullptr),
        _is_master(true),
        _address(0),
        _remote_address(0),
        _slave_intrpt_pin(-1),
        _master_intrpt_await_pin(-1),
        _packet_received_flag(false),
        _tx_seq_counter(0),
        _global_ack_timeout(100),
        _global_max_retries(3)
    {}

    MqttI2C(const MqttI2C&) = delete;
    MqttI2C& operator=(const MqttI2C&) = delete;

    // Статический обработчик onReceive для slave
    static void static_on_receive(int num_bytes) {
        MqttI2C& inst = MqttI2C::getInstance();
        if (num_bytes == sizeof(MqttMessageI2C)) {
            inst._i2c->readBytes(inst._listener_rx_buffer, sizeof(MqttMessageI2C));
            inst._packet_received_flag = true;
        }
    }
    
    // Статический обработчик onRequest для slave
    static void static_on_request() {
        MqttI2C& inst = MqttI2C::getInstance();
        inst._i2c->write(reinterpret_cast<const uint8_t*>(&(inst._pending_message)), sizeof(MqttMessageI2C));
    }

protected:
    // Низкоуровневая отправка сообщения по I2C целым пакетом.
    void send_message_internal(const MqttMessageI2C& msg) {
        if (_is_master) {
            _i2c->beginTransmission(msg.dst);
            _i2c->write(reinterpret_cast<const uint8_t*>(&msg), sizeof(MqttMessageI2C));
            _i2c->endTransmission();
        } else {
            _pending_message = msg;
        }
    }
    
    // State-machine отправки
    void update_send() {
        if (_send_queue.empty()) return;
        MqttOutgoingMessageI2C& current = _send_queue.front();
        unsigned long now = millis();
        if (!current.waiting_ack) {
            current.last_attempt_ms = now;
            current.waiting_ack = true;
            send_message_internal(current.msg);
            if (!_is_master && _slave_intrpt_pin != -1)
                digitalWrite(_slave_intrpt_pin, HIGH); // Уведомляем мастера
        } else {
            if (now - current.last_attempt_ms >= current.ack_timeout_ms) {
                if (current.retries < current.max_retries) {
                    current.retries++;
                    current.last_attempt_ms = now;
                    send_message_internal(current.msg);
                    if (!_is_master && _slave_intrpt_pin != -1)
                        digitalWrite(_slave_intrpt_pin, HIGH);
                } else {
                    current.waiting_ack = false;
                    if (!_is_master && _slave_intrpt_pin != -1)
                        digitalWrite(_slave_intrpt_pin, LOW); // Сброс уведомляющего сигнала
                    if (OUTGOING_MESSAGE_FAILED_PUSH_BACK) {
                        MqttOutgoingMessageI2C failed = current;
                        _send_queue.erase(_send_queue.begin());
                        _send_queue.push_back(failed);
                    }
                }
            }
        }
    }
    
    // State-machine приёма
    void update_receive() {
        if (_is_master) {
            if (_master_intrpt_await_pin != -1 && digitalRead(_master_intrpt_await_pin) == HIGH) {
                MqttMessageI2C msg;
                // Запрашиваем с slave по его адресу (_remote_address)
                uint8_t bytes_read = _i2c->requestFrom(_remote_address, (uint8_t)sizeof(MqttMessageI2C));
                uint8_t* p = reinterpret_cast<uint8_t*>(&msg);
                size_t index = 0;
                while (_i2c->available() && index < sizeof(MqttMessageI2C))
                    p[index++] = _i2c->read();
                process_received_packet(msg);
            }
        } else {
            if (_packet_received_flag) {
                _packet_received_flag = false;
                MqttMessageI2C msg;
                memcpy(&msg, _listener_rx_buffer, sizeof(MqttMessageI2C));
                process_received_packet(msg);
            }
        }
    }
    
    // Обработка полученного пакета
    void process_received_packet(const MqttMessageI2C& msg) {
        if (msg.start != START_DELIMITER || msg.end != END_DELIMITER)
            return;
        uint16_t crc_calc = calculateCRC(reinterpret_cast<const uint8_t*>(&msg),
                            sizeof(msg) - sizeof(msg.crc) - sizeof(msg.end));
        if (crc_calc != msg.crc) {
            send_ack_nack(msg, false);
            return;
        }
        if (msg.cmd == CMD_ACK) {
            if (!_send_queue.empty() && msg.seq == _send_queue.front().msg.seq)
                _send_queue.erase(_send_queue.begin());
            if (!_is_master && _slave_intrpt_pin != -1)
                digitalWrite(_slave_intrpt_pin, LOW);
        }
        else if (msg.cmd == CMD_NACK) {
            if (!_send_queue.empty() && msg.seq == _send_queue.front().msg.seq)
                _send_queue.front().last_attempt_ms = 0;
            if (!_is_master && _slave_intrpt_pin != -1)
                digitalWrite(_slave_intrpt_pin, LOW);
        }
        else {
            send_ack_nack(msg, true);
            if (!_is_master && _slave_intrpt_pin != -1)
                digitalWrite(_slave_intrpt_pin, LOW);
            for (auto& pair : _handlers) {
                if (pair.first == msg.cmd)
                    pair.second(msg);
            }
        }
    }
    
    // Отправка ACK или NACK
    void send_ack_nack(const MqttMessageI2C& rec_msg, bool ack) {
        MqttMessageI2C ack_msg;
        ack_msg.start = START_DELIMITER;
        ack_msg.src   = _address;
        ack_msg.dst   = rec_msg.src;
        ack_msg.cmd   = ack ? CMD_ACK : CMD_NACK;
        ack_msg.seq   = rec_msg.seq;
        ack_msg.len   = 0;
        ack_msg.crc   = calculateCRC(reinterpret_cast<const uint8_t*>(&ack_msg),
                         sizeof(ack_msg) - sizeof(ack_msg.crc) - sizeof(ack_msg.end));
        ack_msg.end   = END_DELIMITER;
        send_message_internal(ack_msg);
    }
    
public:
    static MqttI2C& getInstance() {
        static MqttI2C instance;
        return instance;
    }
    
    // Дополнительный метод для мастера: установка адреса slave, с которого запрашивать данные
    void set_remote_address(uint8_t remote_addr) {
        _remote_address = remote_addr;
    }
    
    // Три перегрузки begin():
    // 1) Инициализация с использованием внутреннего объекта Wire
    virtual I2C_CH* begin(bool is_master, int addr, int notify_pin = -1) override {
        _i2c = &Wire;
        _is_master = is_master;
        _address = addr;
        _slave_intrpt_pin = notify_pin;
        _tx_seq_counter = 0;
        _global_ack_timeout = 100;
        _global_max_retries = 3;
        if (_is_master) {
            _i2c->begin();
        } else {
            _i2c->begin(_address);
            _i2c->onReceive(MqttI2C::static_on_receive);
            _i2c->onRequest(MqttI2C::static_on_request);
            _packet_received_flag = false;
        }
        return _i2c;
    }
    
    // 2) Инициализация с внешне проинициализированным объектом I2C
    virtual I2C_CH* begin(I2C_CH* external_i2c, bool is_master, int addr, int notify_pin = -1) override {
        _is_master = is_master;
        _address = addr;
        _slave_intrpt_pin = notify_pin;
        _tx_seq_counter = 0;
        _global_ack_timeout = 100;
        _global_max_retries = 3;
        _i2c = external_i2c;
        if (!_is_master) {
            _i2c->onReceive(MqttI2C::static_on_receive);
            _i2c->onRequest(MqttI2C::static_on_request);
            _packet_received_flag = false;
        }
        return _i2c;
    }
    
    // 3) Инициализация по пинам (sda, scl, частота)
    virtual I2C_CH* begin(bool is_master, int addr, int sda, int scl, int frequency = 400000, int notify_pin = -1) override {
        _is_master = is_master;
        _address = addr;
        _slave_intrpt_pin = notify_pin;
        _tx_seq_counter = 0;
        _global_ack_timeout = 100;
        _global_max_retries = 3;
#ifdef DEV_SSPS3_IS_MASTER
        _i2c = new I2C_CH(0);
#else
        _i2c = new I2C_CH();
#endif
        if (_is_master) {
            _i2c->begin(sda, scl, frequency);
        } else {
            _i2c->begin(sda, scl, frequency);
            _i2c->onReceive(MqttI2C::static_on_receive);
            _i2c->onRequest(MqttI2C::static_on_request);
            _packet_received_flag = false;
        }
        return _i2c;
    }
    
    virtual void setAddress(uint8_t addr) override {
        _address = addr;
    }
    
    virtual void registerHandler(uint8_t cmd, MqttReceivedI2CCommandHandler handler) override {
        _handlers.push_back({cmd, handler});
    }
    
    virtual void setGlobalAckTimeout(unsigned long timeout) override {
        _global_ack_timeout = timeout;
    }
    
    virtual void setGlobalMaxRetries(uint8_t retries) override {
        _global_max_retries = retries;
    }
    
    virtual void queueMessage(uint8_t dst, uint8_t cmd, const uint8_t* data, uint8_t length,
                              uint8_t max_retries = 0, unsigned long ack_timeout = 0) override {
        if (length > MAX_PAYLOAD_SIZE_I2C)
            return;
        MqttMessageI2C msg;
        msg.start = START_DELIMITER;
        msg.src   = _address;
        msg.dst   = dst;
        msg.cmd   = cmd;
        msg.seq   = _tx_seq_counter++;
        msg.len   = length;
        memcpy(msg.payload, data, length);
        msg.crc   = calculateCRC(reinterpret_cast<const uint8_t*>(&msg),
                    sizeof(msg) - sizeof(msg.crc) - sizeof(msg.end));
        msg.end   = END_DELIMITER;
        
        MqttOutgoingMessageI2C out;
        out.msg = msg;
        out.retries = 0;
        out.last_attempt_ms = 0;
        out.max_retries = (max_retries == 0) ? _global_max_retries : max_retries;
        out.ack_timeout_ms = (ack_timeout == 0) ? _global_ack_timeout : ack_timeout;
        out.waiting_ack = false;
        _send_queue.push_back(out);
    }
    
    virtual void update() override {
        update_send();
        update_receive();
    }
    
    virtual void setSlaveNotifyPin(int pin) override {
        _master_intrpt_await_pin = pin;
        pinMode(_master_intrpt_await_pin, INPUT);
    }
    
    virtual void notifyPacketReceived(const uint8_t* data, size_t length) override {
        if (length != sizeof(MqttMessageI2C))
            return;
        memcpy(_listener_rx_buffer, data, length);
        _packet_received_flag = true;
    }
    
    virtual uint16_t calculateCRC(const uint8_t* data, size_t length) override {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= static_cast<uint16_t>(data[i]) << 8;
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x8000)
                    crc = (crc << 1) ^ 0x1021;
                else
                    crc <<= 1;
            }
        }
        return crc;
    }
};

#endif
