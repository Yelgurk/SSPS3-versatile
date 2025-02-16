#pragma once
#ifndef I_MQTT_SPI_H
#define I_MQTT_SPI_H

#ifdef DEV_SSPS3_RUN_ON_PLC
    #include "./i_arduino_spi.h"
    #include "./i_mqtt.h"
#else
    #include "./i_arduino_spi.h"
    #include "./i_mqtt.h"
#endif

#define OUTGOING_MESSAGE_FAILED_PUSH_BACK   false

static const constexpr uint8_t  START_DELIMITER = 0xAA;
static const constexpr uint8_t  END_DELIMITER = 0x55;

static const constexpr uint8_t  CMD_ACK = 0xFE;
static const constexpr uint8_t  CMD_NACK = 0xFF;

static const constexpr size_t   MAX_PAYLOAD_SIZE_SPI = 32;

//-----------------------------------------------------------------
// Структура MqttMessageSPI – формат передаваемого сообщения
//-----------------------------------------------------------------
#pragma pack(push, 1)
struct MqttMessageSPI
{
    uint8_t start;                          // стартовый делимитер (0xAA)
    uint8_t src;                            // адрес отправителя
    uint8_t dst;                            // адрес получателя
    uint8_t cmd;                            // код команды
    uint8_t seq;                            // порядковый номер сообщения (для контроля повторов)
    uint8_t len;                            // длина полезной нагрузки (значимы только первые len байт массива payload)
    uint8_t payload[MAX_PAYLOAD_SIZE_SPI];  // полезная нагрузка до 32-х байт
    uint16_t crc;                           // CRC-16 (вычисляется по всем байтам, кроме crc и end)
    uint8_t end;                            // конечный делимитер (0x55)
};
#pragma pack(pop)

//-----------------------------------------------------------------
// Структура для хранения информации об исходящем сообщении
//-----------------------------------------------------------------
struct MqttOutgoingMessageSPI
{
    MqttMessageSPI msg;                     // сообщение для отправки
    uint8_t retries;                        // число выполненных попыток
    unsigned long last_attempt;             // время последней отправки (millis)
    uint8_t max_retries;                    // максимальное число повторов (устанавливается через глобальные или локальные параметры)
    unsigned long ack_timeout;              // таймаут ожидания ACK (в мс)
    bool waiting_ack;                       // флаг, что сообщение уже отправлено и ждёт ACK
};

//-----------------------------------------------------------------
// Тип для обработчиков команд: функция, принимающая полученное сообщение
//-----------------------------------------------------------------
using MqttReceivedSPICommandHandler = std::function<void(const MqttMessageSPI&)>;

class IMqttSPI : public IMqtt
{

    virtual SPI_CH* begin(
        bool isMaster,
        int csPin,
        int speed_hz = DEFAULT_SPI_FRQ,
        int notifyPin = -1
    ) = 0;

    virtual SPI_CH* begin(
        SPI_CH* _spi,
        bool isMaster,
        int csPin,
        int speed_hz = DEFAULT_SPI_FRQ,
        int notifyPin = -1
    ) = 0;

    virtual SPI_CH* begin(
        int mosi,
        int miso,
        int sck,
        bool isMaster,
        int csPin,
        uint8_t spi_bus = HSPI,
        int speed_hz = DEFAULT_SPI_FRQ,
        int notifyPin = -1
    ) = 0;

    // Регистрация обработчика для указанного кода команды.
    virtual void registerHandler(
        uint8_t cmd,
        MqttReceivedSPICommandHandler handler
    ) = 0;
};



#endif // !I_MQTT_SPI_H
