#pragma once
#ifndef I_MQTT_I2C_H
#define I_MQTT_I2C_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./i_arduino_i2c.h"
    #include "./i_mqtt.h"
#else
    #include "./i_arduino_i2c.h"
    #include "./i_mqtt.h"
#endif

//-----------------------------------------------------------------
// Структура MqttMessageI2C – формат передаваемого сообщения
//-----------------------------------------------------------------
#pragma pack(push, 1)
struct MqttMessageI2C
{
    uint8_t start;                          // стартовый делимитер (0xAA)
    uint8_t src;                            // адрес отправителя
    uint8_t dst;                            // адрес получателя
    uint8_t cmd;                            // код команды
    uint8_t seq;                            // порядковый номер сообщения (для контроля повторов)
    uint8_t len;                            // длина полезной нагрузки (значимы только первые len байт массива payload)
    uint8_t payload[MAX_PAYLOAD_SIZE_I2C];  // полезная нагрузка до 4-х байт
    uint16_t crc;                           // CRC-16 (вычисляется по всем байтам, кроме crc и end)
    uint8_t end;                            // конечный делимитер (0x55)
};
#pragma pack(pop)

//-----------------------------------------------------------------
// Структура для хранения информации об исходящем сообщении
//-----------------------------------------------------------------
struct MqttOutgoingMessageI2C
{
    MqttMessageI2C  msg;
    uint8_t         retries;
    uint8_t         max_retries;
    unsigned long   last_attempt_ms;
    unsigned long   ack_timeout_ms;
    bool            waiting_ack;
};

//-----------------------------------------------------------------
// Тип для обработчиков команд: функция, принимающая полученное сообщение
//-----------------------------------------------------------------
using MqttReceivedI2CCommandHandler = std::function<void(const MqttMessageI2C&)>;

class IMqttI2C : public IMqtt
{
    virtual I2C_CH* begin(
        bool isMaster,
        int addr,
        int notifyPin = -1
    ) = 0;

    virtual I2C_CH* begin(
        I2C_CH* externalI2C,
        bool isMaster,
        int addr,
        int notifyPin = -1
    ) = 0;

    
    virtual I2C_CH* begin(
        bool isMaster,
        int addr,
        int sda,
        int scl,
        int frequency = 400000,
        int notifyPin = -1
    ) = 0;

    // Регистрация обработчика для указанного кода команды.
    virtual void registerHandler(
        uint8_t cmd,
        MqttReceivedI2CCommandHandler handler
    ) = 0;
};



#endif
