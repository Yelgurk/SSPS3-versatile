#pragma once
#ifndef I_MQTT_H
#define I_MQTT_H

#include <functional>
#include <vector>

#define OUTGOING_MESSAGE_FAILED_PUSH_BACK   false

static const constexpr uint8_t  START_DELIMITER = 0xAA;
static const constexpr uint8_t  END_DELIMITER = 0x55;

static const constexpr uint8_t  CMD_ACK = 0xFE;
static const constexpr uint8_t  CMD_NACK = 0xFF;

static const constexpr size_t   MAX_PAYLOAD_SIZE_SPI = 32;
static const constexpr size_t   MAX_PAYLOAD_SIZE_I2C = 4;

class IMqtt
{
public:
    virtual ~IMqtt() {}

    virtual void setAddress(uint8_t addr) = 0;

    virtual void setGlobalAckTimeout(unsigned long timeout) = 0;
    virtual void setGlobalMaxRetries(uint8_t retries) = 0;

    virtual void queueMessage(uint8_t dst, uint8_t cmd, const uint8_t* data, uint8_t length, uint8_t maxRetries = 0, unsigned long ackTimeout = 0) = 0;
    virtual void update() = 0;

    virtual void setSlaveNotifyPin(int pin) = 0;

    // опцциональный метод: для применения по факту окончания получения "пакета", если реализовать в ISR/DMA свой метод приёма данных (т.е. уходя от onReceived() в Arduino)
    virtual void notifyPacketReceived(const uint8_t* data, size_t length) = 0; 

    virtual uint16_t calculateCRC(const uint8_t* data, size_t length) = 0;
};

#endif // I_MQTT_H
